#include "ModelOBJ.h"

namespace MIS
{

	ModelOBJ::Material::Material(const Material& m)
		: name(m.name)
		, Ns(m.Ns)
		, Ka{ m.Ka[0], m.Ka[1], m.Ka[2] }
		, Kd{ m.Kd[0], m.Kd[1], m.Kd[2] }
		, Ks{ m.Ks[0], m.Ks[1], m.Ks[2] }
		, Ke{ m.Ke[0], m.Ke[1], m.Ke[2] }
		, Ni(m.Ni)
		, d(m.d)
		, illum(m.illum)
		, map_Kd(m.map_Kd)
	{
	}

	ModelOBJ::Material::Material(QString fileName)
		: Ns(0)
		, Ka{0, 0, 0}
		, Kd{0, 0, 0}
		, Ks{0, 0, 0}
		, Ke{0, 0, 0}
		, Ni(0)
		, d(0)
		, illum(0)
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
			ts.seek(0);
			while (!ts.atEnd())
			{
				line = ts.readLine();
				ls.seek(0);
				ls >> element;
				if (element == "newmtl")
				{
					ls >> name;
				}
				else if (element == "Ns")
				{
					ls >> Ns;
				}
				else if (element == "Ka")
				{
					for (unsigned int i = 0; i < 3; i++)
						ls >> Ka[i];
				}
				else if (element == "Kd")
				{
					for (unsigned int i = 0; i < 3; i++)
						ls >> Kd[i];
				}
				else if (element == "Ks")
				{
					for (unsigned int i = 0; i < 3; i++)
						ls >> Ks[i];
				}
				else if (element == "Ke")
				{
					for (unsigned int i = 0; i < 3; i++)
						ls >> Ke[i];
				}
				else if (element == "Ni")
				{
					ls >> Ni;
				}
				else if (element == "d")
				{
					ls >> d;
				}
				else if (element == "illum")
				{
					ls >> illum;
				}
				else if (element == "map_Kd")
				{
					ls >> map_Kd;
					map_Kd = QFileInfo(fileName).path() + "/" + map_Kd;
				}
			}
			file.close();
		}
	}

	ModelOBJ::ModelOBJ(QString _fileName)
		: Model3D(_fileName)
	{
		primitives = TRIANGLES;
		liveLoading = true;
		prepare();
	}

	void ModelOBJ::loadRamThread()
	{
		for (unsigned int i = 0; i < f.count(); i++)
		{
			for (unsigned int j = 0; j < 3; j++)
			{
				pos.append(v[f[i][j][0] - 1]);
				uv.append(vt[f[i][j][1] - 1]);
				normal.append(vn[f[i][j][2] - 1]);
			}
		}

		for (Material& material : materials)
		{
			textures.append(QImage(material.map_Kd).mirrored());
		}
	}

	void ModelOBJ::prepare()
	{
		bool AABBtheFirst = false;
		AABB aabb;
		QTextStream ts(file);
		QString line;
		QString element;
		QTextStream ls(&line);
		ts.seek(0);
		while (!ts.atEnd())
		{
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
				v.append(_v);
			}
			else if (element == "vt")
			{
				vec2 _vt;
				ls >> _vt[0];
				ls >> _vt[1];
				vt.append(_vt);
			}
			else if (element == "vn")
			{
				vec3 _vn;
				for (unsigned int i = 0; i < 3; i++)
					ls >> _vn[i];
				vn.append(_vn);
			}
			else if (element == "f")
			{
				QVector<vec3> separated(3);
				for (unsigned int i = 0; i < 3; i++)
				{
					ls >> element;
					QStringList fl = element.split("/");
					for (unsigned int j = 0; j < 3; j++)
						separated[i][j] = fl[j].toInt();
				}
				f.append(separated);
			}
		}

		vertexNumber = 3 * f.count();
		aabb.gravity /= v.count();
		aabb.updateCenter();
		setAABB(aabb);
		prepared = true;
	}

}