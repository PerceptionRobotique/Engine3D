#include "ModelOCT.h"

ModelOCT::ModelOCT(QString _fileName)
	: Model3D(_fileName)
{
	if (fileInfo.suffix() == "octi") m_hasIntensity = true;

	float aabb[6];
	file.read((char*)vertexNumber, sizeof(long long));
	file.read((char*)aabb, 6 * sizeof(float));

	QFile listOctree(fileInfo.path() + "/listOctree.txt");
	if (listOctree.open(QFile::ReadOnly))
	{
		while (!listOctree.atEnd())
		{
			QString nodeName = listOctree.readLine();
			
		}
		listOctree.close();
	}
}