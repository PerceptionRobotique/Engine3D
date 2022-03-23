#include "Octree.h"

Octree::Octree(Octree* _parent, QString _fileName)
	: ModelBIN(_fileName)
	, main(this)
	, parent(_parent)
	, children(8, nullptr)
	, depth(0)
{
	primitives = POINTS;
	liveLoading = true;

	if (parent)
	{
		main = parent->main;
		depth = parent->depth + 1;
		maxDepth = main->maxDepth;
		if (*maxDepth < depth) *maxDepth = depth;
		m_hasIntensity = parent->hasIntensity();
		//file = main->file;
		file = new QFile(main->file->fileName());
		file->open(QFile::ReadOnly);
		fileMutex = main->fileMutex;
		file->seek(main->file->pos());
		totalVertexNumber = main->totalVertexNumber;
		ModelBIN::prepare();
		main->file->seek(filePos
			+ 3 * vertexNumber * sizeof(float)
			+ 3 * vertexNumber * sizeof(unsigned char)
			+ (hasIntensity() ? vertexNumber * sizeof(unsigned char) : 0)
		);
		*totalVertexNumber += vertexNumber;

		connect(this, SIGNAL(modelChanged()), parent, SIGNAL(modelChanged()));
		connect(this, SIGNAL(modelLoadingDelayed()), parent, SIGNAL(modelLoadingDelayed()));
		connect(this, SIGNAL(modelLoaded()), parent, SIGNAL(modelLoaded()));
		connect(this, SIGNAL(vertexOnRAMChanged(unsigned long long)), parent, SIGNAL(vertexOnRAMChanged(unsigned long long)));
		connect(this, SIGNAL(vertexOnVRAMChanged(unsigned long long)), parent, SIGNAL(vertexOnVRAMChanged(unsigned long long)));
	}
	else
	{
		file->seek(filePos
			+ 3 * vertexNumber * sizeof(float)
			+ 3 * vertexNumber * sizeof(unsigned char)
			+ (hasIntensity() ? vertexNumber * sizeof(unsigned char) : 0)
		);
		fileMutex = new QMutex;
		maxDepth = new unsigned int(0);
		totalVertexNumber = new unsigned long long(vertexNumber);
		listOctree.setFileName(fileInfo.path() + "/listOctree.txt");
		if (listOctree.open(QFile::ReadOnly))
		{
			QString nodeName;
			while(!listOctree.atEnd())
			{
				Octree* currentParent = nullptr;
				Octree* current = this;
				unsigned int childIndex;
				nodeName = listOctree.readLine();
				nodeName.remove('\r').remove('\n');
				for (unsigned int c = 1; c < nodeName.length(); c++)
				{
					childIndex = QString(nodeName[c]).toUInt();
					currentParent = current;
					current = (*current)[childIndex];
				}
				if (current != this)
				{
					current = new Octree(currentParent);
					currentParent->children[childIndex] = current;
					if (childrenByDepth.count() < current->depth) childrenByDepth.append(QVector<Octree*>(0));
					childrenByDepth[current->depth-1].append(current);
				}
			}
			listOctree.close();
		}
	}
}

Octree::~Octree()
{
	for (Octree* child : children) delete child;
	if (main == this)
	{
		delete fileMutex;
		delete maxDepth;
		delete totalVertexNumber;
	}
	//if (main != this) file = nullptr;
}

Octree* Octree::getChild(unsigned int index)
{
	return children[index];
}

QVector<Octree*>& Octree::getChildren()
{
	return children;
}

QVector<Octree*>& Octree::getDepthChildren(unsigned int _depth)
{
	return childrenByDepth[_depth-1];
}

unsigned int Octree::getDepth() const
{
	return depth;
}

unsigned int Octree::getMaxDepth() const
{
	return *maxDepth;
}

Octree* Octree::operator[](std::size_t index)
{
	return children[index];
}

void Octree::loadRAMthread()
{
	//fileMutex->lock();
	ModelBIN::loadRAMthread();
	//fileMutex->unlock();
}

bool Octree::draw(QOpenGLShaderProgram* shader)
{
	setwMo(main->getwMo());
	//if (drawn)
	//{
	//	for (Octree* child : children)
	//		if (child)
	//			child->draw(shader);
	//}
	return Model3D::draw(shader);
}

bool Octree::drawBox(QOpenGLShaderProgram* shader)
{
	setwMo(main->getwMo());
	//if (drawn)
	//{
	//	for (Octree* child : children)
	//		if (child)
	//			child->drawBox(shader);
	//}
	return Model3D::drawBox(shader);
}