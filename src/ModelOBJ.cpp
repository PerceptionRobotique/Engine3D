#include "ModelOBJ.h"

namespace MIS
{

	ModelOBJ::Material::Material()
		: Ns(0)
		, Ka(0)
		, Kd(0)
		, Ks(0)
		, Ke(0)
		, Ni(0)
		, d(0)
		, illum(0)
		, map_Kd("")
	{
	}

	ModelOBJ::Material::Material(const Material& m)
		: Ns(m.Ns)
		, Ka(m.Ka)
		, Kd(m.Kd)
		, Ks(m.Ks)
		, Ke(m.Ke)
		, Ni(m.Ni)
		, d(m.d)
		, illum(m.illum)
		, map_Kd(m.map_Kd)
	{
	}

	void ModelOBJ::openMaterial(QString fileName)
	{
		QFile file(fileName);
		if (file.open(QFile::ReadOnly))
		{
			QTextStream ts(&file);
			QString line;
			QString element;
			QTextStream ls(&line);
			QString currentName;
			ts.seek(0);
			while (!ts.atEnd())
			{
				line = ts.readLine();
				ls.seek(0);
				ls >> element;
				if (element == "newmtl")
				{
					ls >> currentName;
					materials[currentName];
				}
				else if (element == "Ns")
				{
					ls >> materials[currentName].Ns;
				}
				else if (element == "Ka")
				{
					for (unsigned int i = 0; i < 3; i++)
						ls >> materials[currentName].Ka[i];
				}
				else if (element == "Kd")
				{
					for (unsigned int i = 0; i < 3; i++)
						ls >> materials[currentName].Kd[i];
				}
				else if (element == "Ks")
				{
					for (unsigned int i = 0; i < 3; i++)
						ls >> materials[currentName].Ks[i];
				}
				else if (element == "Ke")
				{
					for (unsigned int i = 0; i < 3; i++)
						ls >> materials[currentName].Ke[i];
				}
				else if (element == "Ni")
				{
					ls >> materials[currentName].Ni;
				}
				else if (element == "d")
				{
					ls >> materials[currentName].d;
				}
				else if (element == "illum")
				{
					ls >> materials[currentName].illum;
				}
				else if (element == "map_Kd")
				{
					QString name = line;
					name.remove("map_Kd ");
					name = name.split("\\\\").last().split("/").last();
					materials[currentName].map_Kd = QFileInfo(fileName).path() + "/" + name;
				}
			}
			file.close();
		}
	}

	ModelOBJ::ModelOBJ(QString _fileName, QOpenGLShaderProgram* shader, QOpenGLShaderProgram* boxShader)
		: Model3D(_fileName, shader, boxShader)
		, stopPrepare(false)
	{
		primitives = TRIANGLES;
		liveLoading = true;
		prepareFuture = QtConcurrent::run(&ModelOBJ::prepare, this);
		//prepareFuture.waitForFinished();
	}

	ModelOBJ::~ModelOBJ()
	{
		if (prepareFuture.isRunning())
		{
			stopPrepare = true;
			prepareFuture.waitForFinished();
		}
		if (!prepared)
		{
			prepareMutex.lock();
			prepareMutex.unlock();
		}
	}

	unsigned int ModelOBJ::computeLines(const QString& text)
	{
		return text.count('\n');
	}

	void ModelOBJ::prepare()
	{
		if (!isPrepared())
		{
			prepareMutex.lock();
			unsigned int progress = 0;
			emit modelLoadingUpdate(this, progress);
			bool AABBtheFirst = false;
			AABB aabb;
			QTextStream ts(file);
			QString line;
			QString element;
			QTextStream ls(&line);
			unsigned int currentLineNumber = 0;
			unsigned int nbLines = 0;
			QString currentObjectName = "";
			QString currentMaterialName = "";
			
			QVector<QString> text;
			QString last = "";
			while (!ts.atEnd())
			{
				text.append(last);
				text.last().append(ts.read(BYTES_PER_READ));
				if (!ts.atEnd())
				{
					last = text.last().split('\n').last();
					text.last().remove(last);
				}
			}
			QFuture<unsigned int> lineNumber = QtConcurrent::mapped(text, ModelOBJ::computeLines);
			for (unsigned int nb : lineNumber.results())
				nbLines += nb;

			ts.seek(0);
			while (!ts.atEnd() && !stopPrepare)
			{
				currentLineNumber++;
				if (progress < (unsigned int)(100.0 * currentLineNumber / nbLines))
				{
					progress = 100.0 * currentLineNumber / nbLines;
					emit modelLoadingUpdate(this, progress);
				}

				line = ts.readLine();
				ls.seek(0);
				ls >> element;
				if (element == "mtllib")
				{
					ls >> element;
					QString materialName = "";
					materialName.append(element.split(".")[0]);
					openMaterial(fileInfo.path() + "/" + element);
				}
				else if (element == "o")
				{
					ls >> currentObjectName;
					objectNames.append(currentObjectName);
				}
				else if (element == "v")
				{
					vec3 _v;
					for (unsigned int i = 0; i < 3; i++)
						ls >> _v[i];
					if (AABBtheFirst)
					{
						if (aabb.min.x > _v.x) aabb.min.x = _v.x;
						if (aabb.min.y > _v.y) aabb.min.y = _v.y;
						if (aabb.min.z > _v.z) aabb.min.z = _v.z;

						if (aabb.max.x < _v.x) aabb.max.x = _v.x;
						if (aabb.max.y < _v.y) aabb.max.y = _v.y;
						if (aabb.max.z < _v.z) aabb.max.z = _v.z;

						aabb.gravity += _v;
					}
					else
					{
						AABBtheFirst = true;
						aabb.min = _v;
						aabb.max = _v;
						aabb.gravity += _v;
					}
					v[currentObjectName].append(_v);
				}
				else if (element == "vt")
				{
					vec2 _vt;
					ls >> _vt[0];
					ls >> _vt[1];
					vt[currentObjectName].append(_vt);
				}
				else if (element == "vn")
				{
					vec3 _vn;
					for (unsigned int i = 0; i < 3; i++)
						ls >> _vn[i];
					vn[currentObjectName].append(_vn);
				}
				else if (element == "f")
				{
					QStringList indexes = ls.readLine().split(" ");
					indexes.removeAll("");
					QVector<vec3> separated;
					for (unsigned int i = 0; i < indexes.count(); i++)
					{
						QStringList fl = indexes[i].split("/");
						vec3 vector;
						for (unsigned j = 0; j < 3; j++)
							vector[j] = fl[j].toInt();
						separated.append(vector);
					}
					f[currentObjectName][currentMaterialName].append(separated);
					
					//NON TRIANGLES
					//QStringList indexes = ls.readLine().split(" ");
					//indexes.removeAll("");
					//QVector<vec3> separated(indexes.count());
					//for (unsigned int t = 1; t < indexes.count() - 1; t++)
					//{
					//	separated.clear();
					//	separated.resize(indexes.count());
					//	unsigned int si = 0;
					//	QVector<unsigned int> indices = { 0, t, t + 1};
					//	for (unsigned int i = 0 ; i < indexes.count() ; i++)
					//	{
					//		QStringList fl = indexes[i].split("/");
					//		for (unsigned int j = 0; j < 3; j++)
					//			separated[si][j] = fl[j].toInt();
					//		si++;
					//	}
					//	f[objectNumber - 1].append(separated);
					//}
				}
				else if (element == "usemtl")
				{
					QString tn;
					ls >> tn;
					currentMaterialName = tn;
				}
			}

			if (!stopPrepare)
			{
				vertexNumber = 0;
				for (QHash<QString, QVector<QVector<vec3>>>& fo : f)
					for (QVector<QVector<vec3>>& fom : fo)
						vertexNumber += 3 * fom.count();
				aabb.gravity /= vertexNumber;
				aabb.updateCenter();
				setAABB(aabb);
				emit modelLoadingUpdate(this, 100);
				prepared = true;
			}
			stopPrepare = false;
			prepareMutex.unlock();
		}
	}

	void ModelOBJ::loadRamThread()
	{
		if (isPrepared())
		{
			point.resize(objectNames.count());
			uv.resize(objectNames.count());
			normal.resize(objectNames.count());
			unsigned int vOffset = 0;
			unsigned int vtOffset = 0;
			unsigned int vnOffset = 0;
			for (unsigned int index = 0; index < objectNames.count(); index++)
			{
				QString objectName = objectNames[index];
				for (unsigned int subBlock = 0; subBlock < f[objectName].keys().count(); subBlock++)
				{
					QString materialName = f[objectName].keys()[subBlock];
					materialUsed[objectNames[index]].append(materialName);
					point[index].resize(f[objectName].count());
					uv[index].resize(f[objectName].count());
					normal[index].resize(f[objectName].count());

					for (unsigned int triangle = 0; triangle < f[objectName][materialName].count(); triangle++)
					{
						for (unsigned int triplet = 0; triplet < 3; triplet++)
						{
							point[index][subBlock].append(v[objectName][f[objectName][materialName][triangle][triplet][0] - vOffset - 1]);
							if (f[objectName][materialName][triangle][triplet][1] - vtOffset - 1 >= 0)
								uv[index][subBlock].append(vt[objectName][f[objectName][materialName][triangle][triplet][1] - vtOffset - 1]);
							normal[index][subBlock].append(vn[objectName][f[objectName][materialName][triangle][triplet][2] - vnOffset - 1]);
						}
					}
				}

				//for (unsigned int subBlock = 0; subBlock < f[objectNames[index]].count(); subBlock++)
				//{
				//	for (unsigned int triangle = 0; triangle < f[objectNames[index]][subBlock].count(); triangle++)
				//	{
				//		for (unsigned int triplet = 0; triplet < 3; triplet++)
				//		{
				//			point[index][subBlock].append(v[index][f[index][subBlock][triangle][triplet][0] - vOffset - 1]);
				//			if (f[index][subBlock][triangle][triplet][1] - vtOffset - 1 >= 0)
				//				uv[index][subBlock].append(vt[index][f[index][subBlock][triangle][triplet][1] - vtOffset - 1]);
				//			normal[index][subBlock].append(vn[index][f[index][subBlock][triangle][triplet][2] - vnOffset - 1]);
				//		}
				//	}
				//}
				vOffset += v[objectName].count();
				vtOffset += vt[objectName].count();
				vnOffset += vn[objectName].count();
			}

			for (QString& materialName : materials.keys())
				textures[materialName] = QImage(materials[materialName].map_Kd).mirrored();
		}
	}

	void ModelOBJ::render(QOpenGLFunctions* f)
	{
		for (unsigned int i = 0; i < objectNames.count(); i++)
		{
			for (unsigned int j = 0; j < pointBuffer[i].count(); j++)
			{
				if (pointBuffer[i][j].isCreated())
				{
					pointBuffer[i][j].bind();
					shader->enableAttributeArray("in_vertex");
					shader->setAttributeArray("in_vertex", GL_FLOAT, 0, 3);
					pointBuffer[i][j].release();
				}

				if (normalBuffer[i][j].isCreated())
				{
					normalBuffer[i][j].bind();
					shader->enableAttributeArray("in_normal");
					shader->setAttributeArray("in_normal", GL_FLOAT, 0, 3);
					normalBuffer[i][j].release();
				}

				if (uvBuffer[i][j].isCreated())
				{
					uvBuffer[i][j].bind();
					shader->enableAttributeArray("in_uv");
					shader->setAttributeArray("in_uv", GL_FLOAT, 0, 2);
					uvBuffer[i][j].release();
				}

				shader->setUniformValue("hasTexture", !materials[materialUsed[objectNames[i]][j]].map_Kd.isEmpty());
				f->glUniform3fv(f->glGetUniformLocation(shader->programId(), "faceColor"), 1, value_ptr(materials[materialUsed[objectNames[i]][j]].Kd));
				if (j < materialUsed[objectNames[i]].count())
					f->glBindTexture(GL_TEXTURE_2D, texturesBuffers[materialUsed[objectNames[i]][j]]->textureId());
				else
					f->glBindTexture(GL_TEXTURE_2D, 0);

				if(pointSizeEnabled)
					f->glDrawArrays(GL_POINTS, 0, point[i][j].count());
				f->glDrawArrays(primitives, 0, point[i][j].count());

				shader->disableAttributeArray("in_vertex");
				shader->disableAttributeArray("in_normal");
				shader->disableAttributeArray("in_uv");
			}
		}
	}

}