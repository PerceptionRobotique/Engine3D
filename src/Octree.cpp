#include "Octree.h"

Octree::Octree(Octree* _parent)
	: parent(_parent)
	, children(8, nullptr)
	, depth(0)
{
	if (parent)
	{
		depth = parent->depth + 1;
	}
}