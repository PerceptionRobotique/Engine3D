#ifndef MODELOBJ_H
#define MODELOBJ_H

#include "Model3D.h"

#include <QHash>

#define BYTES_PER_READ 1000000

namespace MIS
{

	class ENGINE3D_EXPORT ModelOBJ : public Model3D
	{
	public:
		struct Material {
			QVector<QString> name;
			QVector<float> Ns;
			QVector<vec3> Ka;
			QVector<vec3> Kd;
			QVector<vec3> Ks;
			QVector<vec3> Ke;
			QVector<float> Ni;
			QVector<float> d;
			QVector<int> illum;
			QHash<QString, QString> map_Kd;

			Material(const Material& m);
			Material(QString fileName = "");
			void openFile(QString fileName);
		};

		ModelOBJ(QString _fileName = "", QOpenGLShaderProgram* shader = nullptr, QOpenGLShaderProgram* boxShader = nullptr);
		~ModelOBJ();

		void loadRamThread() override;

	private:
		QMutex prepareMutex;
		QHash<QString, Material> materials;

		unsigned int objectNumber;
		QVector<unsigned long long> subVertexNumber;
		QVector<QVector<vec3>> v;
		QVector<QVector<vec2>> vt;
		QVector<QVector<vec3>> vn;
		QVector<QVector<QVector<vec3>>> f;
		QVector<QString> textureNames;

		static unsigned int computeLines(const QString& text);
		void prepare();
		void render(QOpenGLFunctions* f) override;
	};

}

#endif // MODELOBJ_H