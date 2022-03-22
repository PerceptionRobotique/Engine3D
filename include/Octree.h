#ifndef OCTREE_H
#define OCTREE_H

#include "Model3D.h"

class ENGINE3D_EXPORT Octree
{
public:
	Octree(Octree* _parent = nullptr);

private:
	Octree* parent;
	QVector<Octree*> children;
	unsigned int depth;
};

#endif // OCTREE_H