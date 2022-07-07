#ifndef OCTREE_H
#define OCTREE_H

#ifdef ANDROID
#include <QFileDialog>
#endif

#include "ModelBIN.h"
#include "Camera.h"

#define ONE_FILE_READER

namespace MIS
{

	class ENGINE3D_EXPORT Octree : public ModelBIN
	{
		Q_OBJECT
	public:
		Octree(Octree* _parent = nullptr, QString _fileName = "", QOpenGLShaderProgram* shader = nullptr, QOpenGLShaderProgram* boxShader = nullptr);
		~Octree();

		Octree* getMainOctree();
		Octree* getChild(unsigned int index);
		QVector<Octree*>& getChildren();
		QVector<Octree*>& getAllChildren();
		QVector<Octree*>& getDepthChildren(unsigned int _depth);
		unsigned int getDepth() const;
		unsigned int getMaxDepth() const;
		unsigned int getMaxVisibleDepth() const;
		unsigned long long getTotalVertexNumber() const;

		mat4 getwMo() const override;
		bool hasIntensity() const override;
		bool getShowIntensity() const override;
		bool isVisible() const override;
		bool isBoxVisible() const override;
		bool isGlobalColorEnabled() const override;
		QColor getGlobalColor() const override;

		glm::vec3 getPosAt(unsigned long long index) override;
		QVector<unsigned char> getColorAt(unsigned long long index) override;
		unsigned char getIntensityAt(unsigned long long index) override;

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

}

#endif // OCTREE_H
