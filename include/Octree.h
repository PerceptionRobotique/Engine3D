#ifndef OCTREE_H
#define OCTREE_H

#include "ModelBIN.h"
#include "Camera.h"

class ENGINE3D_EXPORT Octree : public ModelBIN
{
	Q_OBJECT
public:
	Octree(Octree* _parent = nullptr, QString _fileName = "");
	~Octree();

	Octree* getChild(unsigned int index);
	QVector<Octree*>& getChildren();
	QVector<Octree*>& getDepthChildren(unsigned int _depth);
	unsigned int getMaxDepth() const;

	Octree* operator[](std::size_t index);

	void loadRAMthread() override;
	bool draw(QOpenGLShaderProgram* shader) override;
	bool drawBox(QOpenGLShaderProgram* shader) override;

private:
	Octree* main;
	Octree* parent;
	QVector<Octree*> children;
	QVector<QVector<Octree*>> childrenByDepth;
	unsigned int depth;
	unsigned int* maxDepth;
	unsigned long long* totalVertexNumber;

	QFile listOctree;
	QMutex* fileMutex;
};

#endif // OCTREE_H