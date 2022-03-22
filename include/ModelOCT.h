#ifndef MODELOCT_H
#define MODELOCT_H

#include "Model3D.h"
#include "Octree.h"

class ENGINE3D_EXPORT ModelOCT : public Model3D
{
public:
	ModelOCT(QString _fileName);

private:
	QHash<unsigned int, Model3D*> children;
};

#endif // MODELOCT_H