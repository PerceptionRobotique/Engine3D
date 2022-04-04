#pragma once

#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QRandomGenerator>
#include <QTime>
#include <QVector>
#include <QThread>
#include <QMessageBox>

#include "Model3D.h"
#include "ModelPTS.h"
#include "ModelBIN.h"
#include "Octree.h"

#define PTS_LINES_TO_WRITE 10000000

class ENGINE3D_EXPORT Model3DWriter : public QObject
{
	Q_OBJECT
public:
	Model3DWriter(QObject* parent = nullptr);
	~Model3DWriter();

	struct Vertex {
		QString node;
		Model3D::AABB aabb;
		QVector<glm::vec3> pos;
		QVector<unsigned char> color;
		QVector<unsigned char> intensity;
		unsigned long long vertexNumber() const { return pos.count(); };
		bool hasIntensity() const { return !intensity.isEmpty(); };
		void append(const Vertex& vertex)
		{
			pos.append(vertex.pos);
			color.append(vertex.color);
			intensity.append(vertex.intensity);
		};
	};

	bool write(Model3D* model, QString fileName, unsigned long long vertexPerNode = 10000);
	static void writePTS(QFile* file, unsigned long long vertexNumber, QVector<glm::vec3>* pos, QVector<unsigned char>* color, QVector<unsigned char>* intensity);
	static void writeBIN(QFile* file, unsigned long long vertexNumber, QVector<float> aabb, QVector<glm::vec3>* pos, QVector<unsigned char>* color, QVector<unsigned char>* intensity);
	static void writeOCT(QFile* file, QFileInfo fileInfo, unsigned long long vertexNumber, Model3D::AABB aabb, QVector<glm::vec3>* pos, QVector<unsigned char>* color, QVector<unsigned char>* intensity, unsigned long long vertexPerNode);

private:
	QThread* writer;
	QFile file;
	QVector<glm::vec3> pos;
	QVector<unsigned char> color;
	QVector<unsigned char> intensity;

	static void computeAABB(unsigned int node, Model3D::AABB aabb, Model3D::AABB* outAABB);
	static QVector<float> aabbToVector(const Model3D::AABB& aabb);
	static void computeNode(Vertex* vertex, QVector<Vertex*>* computedVertex);
	static void getVertexInBox(Vertex* vertex, Vertex* outVertex);
	static void takeRandomVertex(unsigned long long vertexNumber, Vertex* vertex, Vertex* outVertex);
	
	template<typename T, typename U>
	static void deleteIndexes(QVector<T>* vector, const QVector<U>* indexes, unsigned int groupSize = 1);
	template<typename T, typename U>
	static void deleteIndex(QVector<T>* vector, const U index, unsigned int groupSize = 1);

private slots:
	void writerFinished();

signals:
	void finished();
};

