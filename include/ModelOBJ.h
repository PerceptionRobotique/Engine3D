#ifndef MODELOBJ_H
#define MODELOBJ_H

#include "Model3D.h"

#include <QHash>

#define BYTES_PER_READ 1000000

namespace MIS
{

	/**
	 * @brief      This class describes a model obj.
	 */
	class ENGINE3D_EXPORT ModelOBJ : public Model3D
	{
	public:
		struct Material {
			float Ns;
			vec3 Ka;
			vec3 Kd;
			vec3 Ks;
			vec3 Ke;
			float Ni;
			float d;
			int illum;
			QString map_Kd;

			Material();
			Material(const Material& m);
		};

		static bool prepareInThread;

		ModelOBJ(QString _fileName = "", QOpenGLShaderProgram* shader = nullptr, QOpenGLShaderProgram* boxShader = nullptr);
		~ModelOBJ();

		void loadRamThread() override;

	private:
		QMutex prepareMutex;
		QHash<QString, Material> materials;
		QFuture<void> prepareFuture;
		bool stopPrepare;

		QVector<QString> objectNames;
		QHash<QString, QVector<vec3>> v;
		QHash<QString, QVector<vec2>> vt;
		QHash<QString, QVector<vec3>> vn;
		QHash<QString, QHash<QString, QVector<QVector<vec3>>>> f; // object / usemtl / triangle / triplet / valeur
		QHash<QString, QVector<QString>> materialUsed;

		void openMaterial(QString fileName);
		static unsigned int computeLines(const QString& text);
		void prepare();
		void render(QOpenGLFunctions* f) override;
	};

}

#endif // MODELOBJ_H