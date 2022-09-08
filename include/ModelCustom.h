#ifndef MODELCUSTOM_H
#define MODELCUSTOM_H

#include "Model3D.h"

namespace MIS
{

	/**
	 * @brief      This class describes a model bin.
	 */
	class ENGINE3D_EXPORT ModelCustom : public Model3D
	{
	public:
		ModelCustom(Primitives primitives, bool liveLoading = false, QOpenGLShaderProgram* shader = nullptr, QOpenGLShaderProgram* boxShader = nullptr);

		virtual void loadRamThread() override;

		QVector<vec3>& getPoints();

		void addVertex(const QVector<vec3>& pos, const QVector<unsigned char>& color, const QVector<unsigned char>& intensity = QVector<unsigned char>());
		void addVertex(const vec3& pos, const QVector<unsigned char>& color, const QVector<unsigned char>& intensity = QVector<unsigned char>());
		void setVertex(const QVector<vec3>& pos, const QVector<unsigned char>& color, const QVector<unsigned char>& intensity = QVector<unsigned char>());
		void removeVertex(unsigned long long index);
		void clear();

	private:
		QVector<vec3> p;
		QVector<unsigned char> c;
		QVector<unsigned char> i;

		void render(QOpenGLFunctions* f) override;
		void clearMemory();
		void updateAABB();
	};

}

#endif // MODELCUSTOM_H