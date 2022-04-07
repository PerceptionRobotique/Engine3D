#ifndef OCTREE_H
#define OCTREE_H

#include "ModelBIN.h"
#include "Camera.h"

#define ONE_FILE_READER

class ENGINE3D_EXPORT Octree : public ModelBIN
{
	Q_OBJECT
public:
	Octree(Octree* _parent = nullptr, QString _fileName = "");
	~Octree();

	Octree* getChild(unsigned int index);
	QVector<Octree*>& getChildren();
	QVector<Octree*>& getAllChildren();
	QVector<Octree*>& getDepthChildren(unsigned int _depth);
	unsigned int getDepth() const;
	unsigned int getMaxDepth() const;
	unsigned int getMaxVisibleDepth() const;
	unsigned long long getTotaleVertexNumber() const;

	mat4 getwMo() const override;
	bool hasIntensity() const override	;
	bool getShowIntensity() const override;
	bool isVisible() const override;
	bool isBoxVisible() const override;
	bool isGlobalColorEnabled() const override;
	QColor getGlobalColor() const override;

	Octree* operator[](std::size_t index);

	void loadRamThread() override;

public slots:
	void setMaxVisibleDepth(int value);

private:
	QString name;
	Octree* main;
	Octree* parent;
	QVector<Octree*> children;
	QVector<Octree*> allChildren;
	QVector<QVector<Octree*>> childrenByDepth;
	unsigned int depth;
	unsigned int* maxDepth;
	unsigned int* maxVisibleDepth;
	unsigned long long* totalVertexNumber;

	QFile listOctree;
	QMutex* fileMutex;
};

#endif // OCTREE_H