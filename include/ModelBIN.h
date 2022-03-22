#ifndef MODELBIN_H
#define MODELBIN_H

#include "Model3D.h"

class ENGINE3D_EXPORT ModelBIN : public Model3D
{
public:
	ModelBIN(QString _fileName);

	void loadRAMthread() override;

protected:
	long long filePos;
};

#endif // MODELBIN_H