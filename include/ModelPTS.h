#ifndef MODELPTS_H
#define MODELPTS_H

#include "Model3D.h"

#include <QFile>
#include <QFileInfo>
#include <functional>

#define BYTES_PER_READ 1000000

class ENGINE3D_EXPORT ModelPTS : public Model3D
{
public:
	ModelPTS(QString _fileName);
	~ModelPTS();

	void loadRamThread() override;

private:
	bool stop;
	QFuture<void> ramLoader;
	QList<QFuture<void>> loaders;
	void computePTSLines(const QStringList& lines, AABB* currentAABB, QVector<glm::vec3>* currentPos, QVector<unsigned char>* currentColor, QVector<unsigned char>* currentIntensity);
};

#endif // MODELPTS_H
