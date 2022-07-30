#include "ModelOBJ.h"

namespace MIS
{

	ModelOBJ::Material::Material(const Material& m)
		: name(m.name)
		, Ns(m.Ns)
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

	ModelOBJ::Material::Material(QString fileName)
	{
		if (!fileName.isEmpty()) openFile(fileName);
	}

	void ModelOBJ::Material::openFile(QString fileName)
	{
		QFile file(fileName);
		if (file.open(QFile::ReadOnly))
		{
			QTextStream ts(&file);
			QString line;
			QString element;
			QTextStream ls(&line);
			unsigned int index = 0;
			ts.seek(0);
			while (!ts.atEnd())
			{
				line = ts.readLine();
				ls.seek(0);
				ls >> element;
				if (element == "newmtl")
				{
					index++;
					name.resize(index);
					Ns.resize(index);
					Ka.resize(index);
					Kd.resize(index);
					Ks.resize(index);
					Ke.resize(index);
					Ni.resize(index);
					d.resize(index);
					illum.resize(index);
					ls >> name[index - 1];
				}
				else if (element == "Ns")
				{
					ls >> Ns[index - 1];
				}
				else if (element == "Ka")
				{
					for (unsigned int i = 0; i < 3; i++)
						ls >> Ka[index - 1][i];
				}
				else if (element == "Kd")
				{
					for (unsigned int i = 0; i < 3; i++)
						ls >> Kd[index - 1][i];
				}
				else if (element == "Ks")
				{
					for (unsigned int i = 0; i < 3; i++)
						ls >> Ks[index - 1][i];
				}
				else if (element == "Ke")
				{
					for (unsigned int i = 0; i < 3; i++)
						ls >> Ke[index - 1][i];
				}
				else if (element == "Ni")
				{
					ls >> Ni[index - 1];
				}
				else if (element == "d")
				{
					ls >> d[index - 1];
				}
				else if (element == "illum")
				{
					ls >> illum[index - 1];
				}
				else if (element == "map_Kd")
				{
					QString name;
					ls >> name;
					map_Kd[this->name[index - 1]] = QFileInfo(fileName).path() + "/" + name;
				}
			}
			file.close();
		}
	}

	ModelOBJ::ModelOBJ(QString _fileName, QOpenGLShaderProgram* shader, QOpenGLShaderProgram* boxShader)
		: Model3D(_fileName, shader, boxShader)
		, objectNumber(0)
		, stopPrepare(false)
	{
		primitives = TRIANGLES;
		liveLoading = true;
		prepareFuture = QtConcurrent::run(&ModelOBJ::prepare, this);
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

	void ModelOBJ::loadRamThread()
	{
		if (isPrepared())
		{
			point.resize(objectNumber);
			uv.resize(objectNumber);
			normal.resize(objectNumber);
			unsigned int vOffset = 0;
			unsigned int vtOffset = 0;
			unsigned int vnOffset = 0;
			for (unsigned int index = 0; index < objectNumber; index++)
			{
				for (unsigned int i = 0; i < f[index].count(); i++)
				{
					for (unsigned int j = 0; j < 3; j++)
					{
						point[index].append(v[index][f[index][i][j][0] - vOffset - 1]);
						uv[index].append(vt[index][f[index][i][j][1] - vtOffset - 1]);
						normal[index].append(vn[index][f[index][i][j][2] - vnOffset - 1]);
					}
				}
				vOffset += v[index].count();
				vtOffset += vt[index].count();
				vnOffset += vn[index].count();
			}

			for (Material& material : materials)
			{
				for (QString& textureName : material.map_Kd.keys())
				{
					textures[textureName] = QImage(material.map_Kd[textureName]).mirrored();
				}
			}
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
			objectNumber = 0;
			unsigned int currentLineNumber = 0;
			unsigned int nbLines = 0;
			
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
					materials[element.split(".")[0]] = Material(fileInfo.path() + "/" + element);
				}
				else if (element == "o")
				{
					if (objectNumber > 0) subVertexNumber[objectNumber - 1] = 3 * f[objectNumber - 1].count();
					objectNumber++;
					subVertexNumber.append(0);
					v.resize(objectNumber);
					vt.resize(objectNumber);
					vn.resize(objectNumber);
					f.resize(objectNumber);
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
					v[objectNumber - 1].append(_v);
				}
				else if (element == "vt")
				{
					vec2 _vt;
					ls >> _vt[0];
					ls >> _vt[1];
					vt[objectNumber - 1].append(_vt);
				}
				else if (element == "vn")
				{
					vec3 _vn;
					for (unsigned int i = 0; i < 3; i++)
						ls >> _vn[i];
					vn[objectNumber - 1].append(_vn);
				}
				else if (element == "f")
				{
					QStringList indexes = ls.readLine().split(" ");
					indexes.removeAll("");
					QVector<vec3> separated(indexes.count());
					for (unsigned int t = 1; t < indexes.count() - 1; t++)
					{
						separated.clear();
						separated.resize(3);
						unsigned int si = 0;
						QVector<unsigned int> indices = { 0, t, t + 1 };
						for (unsigned int i : indices)
						{
							QStringList fl = indexes[i].split("/");
							for (unsigned int j = 0; j < 3; j++)
								separated[si][j] = fl[j].toInt();
							si++;
						}
						f[objectNumber - 1].append(separated);
					}
				}
				else if (element == "usemtl")
				{
					QString tn;
					ls >> tn;
					textureNames.append(tn);
				}
			}

			if (!stopPrepare)
			{
				subVertexNumber[objectNumber - 1] = 3 * f[objectNumber - 1].count();
				vertexNumber = 0;
				for (unsigned long long& vNumber : subVertexNumber)
					vertexNumber += vNumber;
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

	void ModelOBJ::render(QOpenGLFunctions* f)
	{
		for (unsigned int i = 0; i < objectNumber; i++)
		{
			if (pointBuffer[i].isCreated())
			{
				pointBuffer[i].bind();
				shader->enableAttributeArray("in_vertex");
				shader->setAttributeArray("in_vertex", GL_FLOAT, 0, 3);
				pointBuffer[i].release();
			}

			if (normalBuffer[i].isCreated())
			{
				normalBuffer[i].bind();
				shader->enableAttributeArray("in_normal");
				shader->setAttributeArray("in_normal", GL_FLOAT, 0, 3);
				normalBuffer[i].release();
			}

			if (uvBuffer[i].isCreated())
			{
				uvBuffer[i].bind();
				shader->enableAttributeArray("in_uv");
				shader->setAttributeArray("in_uv", GL_FLOAT, 0, 2);
				uvBuffer[i].release();
			}

			if (texturesBuffers.keys().contains(textureNames[i]))
				f->glBindTexture(GL_TEXTURE_2D, texturesBuffers[textureNames[i]]->textureId());
			else
				f->glBindTexture(GL_TEXTURE_2D, 0);

			f->glDrawArrays(primitives, 0, subVertexNumber[i]);

			shader->disableAttributeArray("in_vertex");
			shader->disableAttributeArray("in_normal");
			shader->disableAttributeArray("in_uv");
		}
	}

}