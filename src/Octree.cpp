#include "Octree.h"

namespace MIS
{

	Octree::Octree(Octree* _parent, QString _fileName, QOpenGLShaderProgram* shader, QOpenGLShaderProgram* boxShader)
		: ModelBIN(_fileName, shader, boxShader)
		, main(this)
		, parent(_parent)
		, children(8, nullptr)
		, depth(0)
	{
		primitives = POINTS;
		liveLoading = true;

		if (parent)
		{
			main = parent->main;
			depth = parent->depth + 1;
			maxDepth = main->maxDepth;
			maxVisibleDepth = main->maxVisibleDepth;
			if (*maxDepth < depth) *maxDepth = depth;
			m_hasIntensity = parent->hasIntensity();
#ifdef ONE_FILE_READER
			file = main->file;
#else
			file = new QFile(main->file->fileName());
			file->open(QFile::ReadOnly);
#endif
			fileMutex = main->fileMutex;
			file->seek(main->file->pos());
			totalVertexNumber = main->totalVertexNumber;
			ModelBIN::prepare();
			main->file->seek(filePos
				+ 3 * vertexNumber * sizeof(float)
				+ 3 * vertexNumber * sizeof(unsigned char)
				+ (hasIntensity() ? vertexNumber * sizeof(unsigned char) : 0)
			);
			*totalVertexNumber += vertexNumber;
		}
		else
		{
			file->seek(filePos
				+ 3 * vertexNumber * sizeof(float)
				+ 3 * vertexNumber * sizeof(unsigned char)
				+ (hasIntensity() ? vertexNumber * sizeof(unsigned char) : 0)
			);
			//file->seek(0);
			//ModelBIN::prepare();
			fileMutex = new QMutex;
			maxDepth = new unsigned int(0);
			maxVisibleDepth = new unsigned int;
			totalVertexNumber = new unsigned long long(vertexNumber);
#ifndef ANDROID
			listOctree.setFileName(fileInfo.path() + "/listOctree.txt");
#else
            QStringList androidPathElements = _fileName.split("%");
            androidPathElements.removeLast();
            QString listOctreeFileName;
            for(const QString& element : androidPathElements)
                listOctreeFileName.append(element + "%");
            listOctree.setFileName(listOctreeFileName + "2FlistOctree.txt");
            while(!listOctree.open(QFile::ReadOnly))
                QFileDialog::getOpenFileName(nullptr, "Ouverture du fichier listOctree.txt", listOctree.fileName(), "listOctree.txt");
            listOctree.close();
#endif
			if (listOctree.open(QFile::ReadOnly))
			{
				QString nodeName;
				while (!listOctree.atEnd())
				{
					Octree* currentParent = nullptr;
					Octree* current = this;
					unsigned int childIndex;
					nodeName = listOctree.readLine();
					nodeName.remove('\r').remove('\n');
					for (unsigned int c = 1; c < nodeName.length(); c++)
					{
						childIndex = QString(nodeName[c]).toUInt();
						currentParent = current;
						current = (*current)[childIndex];
					}
					if (current != this)
					{
						current = new Octree(currentParent, "", shader, boxShader);

						connect(current, SIGNAL(modelChanged()), this, SIGNAL(modelChanged()));
						connect(current, SIGNAL(modelLoadingDelayed()), this, SIGNAL(modelLoadingDelayed()));
						connect(current, SIGNAL(modelLoaded()), this, SIGNAL(modelLoaded()));
						connect(current, SIGNAL(modelUnloaded()), this, SIGNAL(modelUnloaded()));
						connect(current, SIGNAL(vertexOnRAMChanged()), this, SIGNAL(vertexOnRAMChanged()));
						connect(current, SIGNAL(vertexOnVRAMChanged()), this, SIGNAL(vertexOnVRAMChanged()));

						current->name = nodeName;
						currentParent->children[childIndex] = current;
						if (childrenByDepth.count() < current->depth) childrenByDepth.append(QVector<Octree*>(0));
						childrenByDepth[current->depth - 1].append(current);
						allChildren.append(current);
					}
				}
				listOctree.close();

				*maxVisibleDepth = *maxDepth;
			}
            else qDebug() << ">>>>>>>>>>>>>>>>>>>>>> CAN'T OPEN LIST OCTREE";
		}
	}

	Octree::~Octree()
	{
		for (Octree* child : children) delete child;
		if (file != main->file) file->close();
		if (main == this)
		{
			delete fileMutex;
			delete maxDepth;
			delete maxVisibleDepth;
			delete totalVertexNumber;
		}
#ifdef ONE_FILE_READER
		if (main != this) file = nullptr;
#endif
	}

	Octree* Octree::getMainOctree()
	{
		return main;
	}

	Octree* Octree::getChild(unsigned int index)
	{
		return children[index];
	}

	QVector<Octree*>& Octree::getChildren()
	{
		return children;
	}

	QVector<Octree*>& Octree::getAllChildren()
	{
		return main->allChildren;
	}

	QVector<Octree*>& Octree::getDepthChildren(unsigned int _depth)
	{
		return childrenByDepth[_depth - 1];
	}

	unsigned int Octree::getDepth() const
	{
		return depth;
	}

	unsigned int Octree::getMaxDepth() const
	{
		return *maxDepth;
	}

	unsigned int Octree::getMaxVisibleDepth() const
	{
		return *maxVisibleDepth;
	}

	unsigned long long Octree::getTotalVertexNumber() const
	{
		return *totalVertexNumber;
	}

	float Octree::getScale() const
	{
		return main->ModelBIN::getScale();
	}

	bool Octree::isPointSizeEnabled() const
	{
		return main->ModelBIN::isPointSizeEnabled();
	}

	float Octree::getPointSize() const
	{
		return main->ModelBIN::getPointSize();
	}

	vec3 Octree::getAxisModifier() const
	{
		return main->ModelBIN::getAxisModifier();
	}

	mat4 Octree::getwMo() const
	{
		return main->ModelBIN::getwMo();
	}

	bool Octree::hasIntensity() const
	{
		return main->ModelBIN::hasIntensity();
	}

	bool Octree::getShowIntensity() const
	{
		return main->ModelBIN::getShowIntensity();
	}

	bool Octree::isVisible() const
	{
		return main->ModelBIN::isVisible();
	}

	bool Octree::isBoxVisible() const
	{
		return main->ModelBIN::isBoxVisible();
	}

	bool Octree::isGlobalColorEnabled() const
	{
		return main->ModelBIN::isGlobalColorEnabled();
	}

	QColor Octree::getGlobalColor() const
	{
		return main->ModelBIN::getGlobalColor();
	}

	glm::vec3 Octree::getPosAt(unsigned long long index)
	{
		if (main == this)
		{
			if (index >= vertexNumber)
			{
				index -= vertexNumber;
				unsigned int i = 0;
				while (index >= allChildren[i]->getVertexNumber())
					index -= allChildren[i]->getVertexNumber();
				return allChildren[i]->getPosAt(index);
			}
			else
			{
				return ModelBIN::getPosAt(index);
			}
		}
		else
		{
			return ModelBIN::getPosAt(index);
		}
	}

	QVector<unsigned char> Octree::getColorAt(unsigned long long index)
	{
		if (main == this)
		{
			if (index >= vertexNumber)
			{
				index -= vertexNumber;
				unsigned int i = 0;
				while (index >= allChildren[i]->getVertexNumber())
					index -= allChildren[i]->getVertexNumber();
				return allChildren[i]->getColorAt(index);
			}
			else
			{
				return ModelBIN::getColorAt(index);
			}
		}
		else
		{
			return ModelBIN::getColorAt(index);
		}
	}

	unsigned char Octree::getIntensityAt(unsigned long long index)
	{
		if (hasIntensity())
		{
			if (main == this)
			{
				if (index >= vertexNumber)
				{
					index -= vertexNumber;
					unsigned int i = 0;
					while (index >= allChildren[i]->getVertexNumber())
						index -= allChildren[i]->getVertexNumber();
					return allChildren[i]->getIntensityAt(index);
				}
				else
				{
					return ModelBIN::getIntensityAt(index);
				}
			}
			else
			{
				return ModelBIN::getIntensityAt(index);
			}
		}
		else return 0;
	}

	bool Octree::isOpacityEnabled() const
	{
		return main->ModelBIN::isOpacityEnabled();
	}

	float Octree::getOpacity() const
	{
		return main->ModelBIN::getOpacity();
	}

	ModelBIN::BlendFunction Octree::getBlendFunction() const
	{
		return main->ModelBIN::getBlendFunction();
	}

	Octree* Octree::operator[](std::size_t index)
	{
		return children[index];
	}

	void Octree::loadRamThread()
	{
#ifdef ONE_FILE_READER
		fileMutex->lock();
#endif
		ModelBIN::loadRamThread();
#ifdef ONE_FILE_READER
		fileMutex->unlock();
#endif
	}

	void Octree::setMaxVisibleDepth(int value)
	{
		*maxVisibleDepth = value;
		emit modelChanged();
	}

	bool Octree::drawBox()
	{
		if (depth == 0) return ModelBIN::drawBox();
		else if (parent->isOnVRAM()) return ModelBIN::drawBox();
		else return false;
	}
}
