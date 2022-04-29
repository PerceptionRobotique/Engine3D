#ifndef MODELBIN_H
#define MODELBIN_H

#include "Model3D.h"

namespace MIS
{

	class ENGINE3D_EXPORT ModelBIN : public Model3D
	{
	public:
		ModelBIN(QString _fileName = "");

		virtual void loadRamThread() override;

	protected:
		void prepare();

	protected:
		long long filePos;
	};

}

#endif // MODELBIN_H