#ifndef MODELBIN_H
#define MODELBIN_H

#include "Model3D.h"

namespace MIS
{

	class ENGINE3D_EXPORT ModelBIN : public Model3D
	{
	public:
		ModelBIN(QString _fileName = "", QOpenGLShaderProgram* shader = nullptr, QOpenGLShaderProgram* boxShader = nullptr);

		virtual void loadRamThread() override;

	protected:
		void prepare();
		void render(QOpenGLFunctions* f) override;

	protected:
		long long filePos;
	};

}

#endif // MODELBIN_H