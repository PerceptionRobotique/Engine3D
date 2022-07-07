#ifndef MODELPTS_H
#define MODELPTS_H

#include "Model3D.h"

#include <QFile>
#include <QFileInfo>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>

#define BYTES_PER_READ 1000000

namespace MIS
{

	class ENGINE3D_EXPORT ModelPTS : public Model3D
	{
	public:
		ModelPTS(QString _fileName, QOpenGLShaderProgram* shader = nullptr, QOpenGLShaderProgram* boxShader = nullptr);
		~ModelPTS();

		void loadRamThread() override;

	private:
		bool stop;
		QFuture<void> ramLoader;
		QList<QFuture<void>> loaders;
		void computePTSLines(const QStringList& lines, AABB* currentAABB, QVector<glm::vec3>* currentPos, QVector<unsigned char>* currentColor, QVector<unsigned char>* currentIntensity);
		void render(QOpenGLFunctions* f) override;
	};

}

#endif // MODELPTS_H
