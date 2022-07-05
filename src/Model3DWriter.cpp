#include "Model3DWriter.h"

namespace MIS
{

	Model3DWriter::Model3DWriter(QObject* parent)
		: QObject(parent)
		, watcher(this)
	{
	}

	Model3DWriter::~Model3DWriter()
	{
	}

	bool Model3DWriter::write(Model3D* model, QString fileName, unsigned long long vertexPerNode, float factor)
	{
		bool ok = true;
		file.setFileName(fileName);
		QFileInfo fileInfo(fileName);
		if (file.open(QFile::WriteOnly))
		{
			unsigned long long vertexNumber = model->getVertexNumber();
			QVector<float> aabb(6);

			Octree* octree = dynamic_cast<Octree*>(model);
			if (octree) vertexNumber = octree->getTotalVertexNumber();

			for (unsigned int i = 0; i < 3; i++) aabb[i] = model->getAABB().min[i];
			for (unsigned int i = 0; i < 3; i++) aabb[3 + i] = model->getAABB().max[i];

			bool isOnRAM = model->isOnRAM();
			model->loadRAM();
			pos.append(model->getPos());
			color.append(model->getColor());
			intensity.append(model->getIntensity());
			if (!isOnRAM) model->unloadRAM();
			if (octree)
			{
				for (Octree* child : octree->getAllChildren())
				{
					isOnRAM = child->isOnRAM();
					child->loadRAM();
					pos.append(child->getPos());
					color.append(child->getColor());
					intensity.append(child->getIntensity());
					if (!isOnRAM) child->unloadRAM();
				}
			}

			if (fileInfo.suffix() == "pts")										writer = QtConcurrent::run(&Model3DWriter::writePTS, &file, vertexNumber, &pos, &color, &intensity);
			else if (fileInfo.suffix() == "bin" || fileInfo.suffix() == "bini")	writer = QtConcurrent::run(&Model3DWriter::writeBIN, &file, vertexNumber, aabb, &pos, &color, &intensity);
			else if (fileInfo.suffix() == "oct" || fileInfo.suffix() == "octi") writer = QtConcurrent::run(&Model3DWriter::writeOCT, &file, fileInfo, vertexNumber, model->getAABB(), &pos, &color, &intensity, vertexPerNode, factor);
			connect(&watcher, SIGNAL(finished()), this, SLOT(writerFinished()));
			watcher.setFuture(writer);
		}
		else ok = false;
		return ok;
	}

	void Model3DWriter::writePTS(QFile* file, unsigned long long vertexNumber, QVector<glm::vec3>* pos, QVector<unsigned char>* color, QVector<unsigned char>* intensity)
	{
		QString out;
		QTextStream fs(file);
		QTextStream ts(&out);
		ts << vertexNumber;
		unsigned long long nbLines = 0;
		for (unsigned long long vertex = 0; vertex < vertexNumber; vertex++)
		{
			ts << Qt::endl << pos->at(vertex).x << ' ' << pos->at(vertex).y << ' ' << pos->at(vertex).z;
			if (!intensity->isEmpty()) ts << ' ' << intensity->at(vertex);
			ts << ' ' << color->at(3 * vertex) << ' ' << color->at(3 * vertex + 1) << ' ' << color->at(3 * vertex + 2);
			nbLines++;
			if (nbLines >= PTS_LINES_TO_WRITE)
			{
				fs << out;
				out.clear();
				nbLines = 0;
				qDebug() << (unsigned int)(100.0f * (vertex + 1.0f) / vertexNumber) << '%';
			}
		}
		fs << out;
	}

	void Model3DWriter::writeBIN(QFile* file, unsigned long long vertexNumber, QVector<float> aabb, QVector<glm::vec3>* pos, QVector<unsigned char>* color, QVector<unsigned char>* intensity)
	{
		bool wasFileOpen = file->isOpen();
		if (!wasFileOpen) file->open(QFile::Append);
		file->write((char*)&vertexNumber, sizeof(unsigned long long));
		file->write((char*)aabb.constData(), sizeof(float[6]));
		file->write((char*)pos->constData(), pos->count() * sizeof(glm::vec3));
		file->write((char*)color->constData(), color->count() * sizeof(unsigned char));
		file->write((char*)intensity->constData(), intensity->count() * sizeof(unsigned char));
		if (!wasFileOpen) file->close();
	}

	void Model3DWriter::writeOCT(QFile* file, QFileInfo fileInfo, unsigned long long vertexNumber, Model3D::AABB aabb, QVector<glm::vec3>* pos, QVector<unsigned char>* color, QVector<unsigned char>* intensity, unsigned long long vertexPerNode, float factor)
	{
		QFile listOctree(fileInfo.path() + "/listOctree.txt");
		QDir(fileInfo.path()).mkdir("octTemp");
		if (listOctree.open(QFile::WriteOnly))
		{
			QTextStream los(&listOctree);
			QMap<QString, QFile*> vertexToFile;
			QVector<Vertex*> vertexToCompute;
			QVector<QVector<Vertex*>*> computedVertex;
			QVector<unsigned int> tree;
			QVector<QThread*> threads;
			unsigned int depth = 0;

			vertexToCompute.append(new Vertex);
			vertexToCompute.last()->node = "r";
			vertexToCompute.last()->aabb = aabb;

			vertexToCompute.last()->pos = *pos;
			pos->clear();
			vertexToCompute.last()->color = *color;
			color->clear();
			vertexToCompute.last()->intensity = *intensity;
			intensity->clear();

			Vertex* firstVertexToFile = new Vertex;
			QVector<float>* firstAABB = new QVector<float>(aabbToVector(aabb));
			takeRandomVertex(qMin(vertexPerNode, vertexNumber), vertexToCompute.first(), firstVertexToFile);
			writeBIN(file, firstVertexToFile->vertexNumber(), *firstAABB, &firstVertexToFile->pos, &firstVertexToFile->color, &firstVertexToFile->intensity);
			vertexNumber -= firstVertexToFile->vertexNumber();
			delete firstVertexToFile;
			delete firstAABB;
			los << 'r';

			while (vertexNumber > 0)
			{
				QVector<Vertex*> nextToCompute;
				unsigned long long computed = 0;
				for (Vertex* vertex : vertexToCompute)
				{
					if (vertex->vertexNumber() > 0)
					{
						if (threads.count() >= QThread::idealThreadCount())
						{
							threads.first()->wait();
							delete threads.first();
							threads.removeFirst();

							qDebug() << depth << " : " << ++computed << "/" << vertexToCompute.count() << "  " << vertexNumber;

							for (Vertex* vertexToStore : *computedVertex.first())
							{
								Vertex vertexToWrite;
								takeRandomVertex(qMin(vertexPerNode, vertexToStore->vertexNumber()), vertexToStore, &vertexToWrite);

								QVector<float> aabbToFile = aabbToVector(vertexToStore->aabb);
								QFile file(fileInfo.path() + "/octTemp/" + vertexToStore->node + ".bin");
								file.open(QFile::WriteOnly);
								writeBIN(&file, vertexToWrite.vertexNumber(), aabbToFile, &vertexToWrite.pos, &vertexToWrite.color, &vertexToWrite.intensity);
								file.close();

								vertexToFile[vertexToStore->node] = new QFile(file.fileName());
								vertexNumber -= vertexToWrite.vertexNumber();
							}
							if (computedVertex.first()->count() > 0)
								nextToCompute.append(*computedVertex.first());
							//for (Vertex* vertex : *computedVertex.first()) delete vertex;
							//delete computedVertex.first();
							computedVertex.removeFirst();
						}

						computedVertex.append(new QVector<Vertex*>);
						threads.append(QThread::create(&Model3DWriter::computeNode, vertex, computedVertex.last()));
						threads.last()->start();
					}
				}

				for (QThread* thread : threads)
				{
					threads.first()->wait();
					delete threads.first();
					threads.removeFirst();

					qDebug() << depth << " : " << ++computed << "/" << vertexToCompute.count() << "  " << vertexNumber;

					for (Vertex* vertexToStore : *computedVertex.first())
					{
						Vertex vertexToWrite;
						takeRandomVertex(qMin(vertexPerNode, vertexToStore->vertexNumber()) * qPow(factor, depth), vertexToStore, &vertexToWrite);

						QVector<float> aabbToFile = aabbToVector(vertexToStore->aabb);
						QFile file(fileInfo.path() + "/octTemp/" + vertexToStore->node + ".bin");
						file.open(QFile::WriteOnly);
						writeBIN(&file, vertexToWrite.vertexNumber(), aabbToFile, &vertexToWrite.pos, &vertexToWrite.color, &vertexToWrite.intensity);
						file.close();

						vertexToFile[vertexToStore->node] = new QFile(file.fileName());
						vertexNumber -= vertexToWrite.vertexNumber();
					}
					if (computedVertex.first()->count() > 0)
						nextToCompute.append(*computedVertex.first());
					//for (Vertex* vertex : *computedVertex.first()) delete vertex;
					//delete computedVertex.first();
					computedVertex.removeFirst();
				}

				for (Vertex* vertexToDelete : vertexToCompute) delete vertexToDelete;
				vertexToCompute.clear();
				vertexToCompute = nextToCompute;

				depth++;
			}

			for (QString node : vertexToFile.keys())
				los << Qt::endl << node;

			for (QFile* vertexFile : vertexToFile)
			{
				vertexFile->open(QFile::ReadOnly);
				file->write(vertexFile->readAll());
				vertexFile->close();
				delete vertexFile;
			}

			listOctree.close();
			QDir(fileInfo.path() + "/octTemp").removeRecursively();
		}
	}

	QFutureWatcher<void>& Model3DWriter::getWatcher()
	{
		return watcher;
	}

	void Model3DWriter::computeAABB(unsigned int node, Model3D::AABB aabb, Model3D::AABB* outAABB)
	{
		outAABB->min.x = (node % 2 == 0 ? aabb.min.x : aabb.center.x);
		outAABB->min.y = (node < 4 ? aabb.min.y : aabb.center.y);
		outAABB->min.z = (node % 4 < 2 ? aabb.min.z : aabb.center.z);

		outAABB->max.x = (node % 2 == 1 ? aabb.max.x : aabb.center.x);
		outAABB->max.y = (node >= 4 ? aabb.max.y : aabb.center.y);
		outAABB->max.z = (node % 4 >= 2 ? aabb.max.z : aabb.center.z);

		outAABB->updateCenter();
	}

	QVector<float> Model3DWriter::aabbToVector(const Model3D::AABB& aabb)
	{
		QVector<float> vAABB = {
			aabb.min.x, aabb.min.y, aabb.min.z,
			aabb.max.x, aabb.max.y, aabb.max.z
		};
		return vAABB;
	}

	void Model3DWriter::computeNode(Vertex* vertex, QVector<Vertex*>* computedVertex)
	{
		for (unsigned int i = 0; i < 8; i++)
		{
			Vertex vertexInBox;
			computeAABB(i, vertex->aabb, &vertexInBox.aabb);
			getVertexInBox(vertex, &vertexInBox);
			if (vertexInBox.vertexNumber() > 0)
			{
				computedVertex->append(new Vertex);
				computedVertex->last()->node = vertex->node + QString::number(i);
				computedVertex->last()->aabb = vertexInBox.aabb;
				computedVertex->last()->append(vertexInBox);
			}
		}
	}

	void Model3DWriter::getVertexInBox(Vertex* vertex, Vertex* outVertex)
	{
		outVertex->pos.clear();
		outVertex->color.clear();
		outVertex->intensity.clear();
		QVector<unsigned long long> indexes;
		for (unsigned long long i = 0; i < vertex->vertexNumber(); i++)
		{
			if (outVertex->aabb.pointInBox(vertex->pos.at(i)))
			{
				indexes.append(i);
				outVertex->pos.append(vertex->pos.at(i));
				for (unsigned int j = 0; j < 3; j++) outVertex->color.append(vertex->color.at(3 * i + j));
				if (vertex->hasIntensity())
					outVertex->intensity.append(vertex->intensity.at(i));
			}
		}
		deleteIndexes(&vertex->pos, &indexes);
		deleteIndexes(&vertex->color, &indexes, 3);
		if (vertex->hasIntensity()) deleteIndexes(&vertex->intensity, &indexes);
	}

	void Model3DWriter::takeRandomVertex(unsigned long long vertexNumber, Vertex* vertex, Vertex* outVertex)
	{
		static QRandomGenerator rg(QTime::currentTime().msec());
		outVertex->pos.clear();
		outVertex->color.clear();
		outVertex->intensity.clear();
		for (unsigned long long i = 0; i < vertexNumber; i++)
		{
			unsigned long long vertexChoosen = rg.bounded((unsigned long long)0, vertex->vertexNumber());
			outVertex->pos.append(vertex->pos.at(vertexChoosen));
			for (unsigned j = 0; j < 3; j++) outVertex->color.append(vertex->color.at(3 * vertexChoosen + j));
			if (vertex->hasIntensity())
				outVertex->intensity.append(vertex->intensity.at(vertexChoosen));

			deleteIndex(&vertex->pos, vertexChoosen);
			deleteIndex(&vertex->color, vertexChoosen, 3);
			if (vertex->hasIntensity()) deleteIndex(&vertex->intensity, vertexChoosen);
		}
		//deleteIndexes(pos, &indexes);
		//deleteIndexes(color, &indexes, 3);
		//if (!intensity->isEmpty()) deleteIndexes(intensity, &indexes);
	}

	template<typename T, typename U>
	void Model3DWriter::deleteIndexes(QVector<T>* vector, const QVector<U>* indexes, unsigned int groupSize)
	{
		//for (unsigned int i = 0; i < indexes->count(); i++)
			//for (unsigned int j = 0; j < groupSize; j++)
				//vector->swapItemsAt(indexes->at(i) + j, vector->count() - (indexes->count() - i) * groupSize + j);
		//vector->remove(vector->count() - indexes->count() * groupSize, indexes->count() * groupSize);
		if (indexes->count() > 0)
		{
			for (unsigned long long i = indexes->count() - 1; i > 0; i--) deleteIndex(vector, indexes->at(i), groupSize);
			deleteIndex(vector, indexes->at(0), groupSize);
		}
	}

	template<typename T, typename U>
	void Model3DWriter::deleteIndex(QVector<T>* vector, const U index, unsigned int groupSize)
	{
		for (unsigned int i = 0; i < groupSize; i++)
			vector->swapItemsAt(groupSize * index + i, vector->count() - groupSize + i);
		vector->remove(vector->count() - groupSize, groupSize);
	}

	void Model3DWriter::writerFinished()
	{
		disconnect(&watcher, SIGNAL(finished()), this, SLOT(writerFinished()));
		file.close();
		emit finished();
	}

}