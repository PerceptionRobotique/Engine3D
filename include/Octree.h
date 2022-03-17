#ifndef OCTREE_H
#define OCTREE_H

#include "Model3D.h"

class Octree : public Model3D
{
	Octree(Octree* _parent = nullptr, QString _fileName = "");

private:
	Octree* first;
	Octree* parent;
	unsigned int depth;

	QVector<Octree*> children;
};

#endif // OCTREE_H