#ifndef MODELPTS_H
#define MODELPTS_H

#include "Model3D.h"

#include <QFile>
#include <QFileInfo>
#include <QThread>

#define BYTES_PER_READ 1000000

class ENGINE3D_EXPORT ModelPTS : public Model3D
{
public:
	ModelPTS(QString _fileName);
	~ModelPTS();

	void loadRAMthread() override;

private:
	void computePTSLines(const QStringList& lines, AABB* currentAABB, QVector<glm::vec3>* currentPos, QVector<unsigned char>* currentColor, QVector<unsigned char>* currentIntensity);
};

#endif // MODELPTS_H
