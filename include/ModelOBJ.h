#ifndef MODELOBJ_H
#define MODELOBJ_H

#include "Model3D.h"

#include <QHash>

namespace MIS
{

	class ENGINE3D_EXPORT ModelOBJ : public Model3D
	{
	public:
		struct Material {
			QString name;
			float Ns;
			float Ka[3];
			float Kd[3];
			float Ks[3];
			float Ke[3];
			float Ni;
			float d;
			int illum;
			QString map_Kd;

			Material(const Material& m);
			Material(QString fileName = "");
			void openFile(QString fileName);
		};

		ModelOBJ(QString _fileName = "");

		void loadRamThread() override;

	private:
		QHash<QString, Material> materials;

		QVector<vec3> v;
		QVector<vec2> vt;
		QVector<vec3> vn;
		QVector<QVector<vec3>> f;

		void prepare();
	};

}

#endif // MODELOBJ_H