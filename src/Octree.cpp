#include "Octree.h"

Octree::Octree(Octree* _parent, QString _fileName)
	: Model3D(_fileName)
	, first(this)
	, parent(_parent)
	, depth(0)
	, children(8, nullptr)
{
	if (parent != nullptr)
	{
		first = parent->first;
		depth = parent->depth + 1;
	}
}