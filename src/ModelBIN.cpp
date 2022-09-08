#include "ModelBIN.h"

namespace MIS
{

	ModelBIN::ModelBIN(QString _fileName, QOpenGLShaderProgram* shader, QOpenGLShaderProgram* boxShader)
		: Model3D(_fileName, shader, boxShader)
	{
		primitives = POINTS;
		liveLoading = true;

		if (file != nullptr)
		{
			prepare();
		}
	}

	void ModelBIN::prepare()
	{
		if (fileInfo.suffix().endsWith('i')) m_hasIntensity = true;

		float _aabb[6];
		AABB aabb;
		file->read((char*)&vertexNumber, sizeof(long long));
		file->read((char*)&_aabb, sizeof(float[6]));
		aabb.min = vec3(_aabb[0], _aabb[1], _aabb[2]);
		aabb.max = vec3(_aabb[3], _aabb[4], _aabb[5]);
		aabb.center = (aabb.max + aabb.min) / 2.0f;
		aabb.gravity = aabb.center;
		setAABB(aabb);

		filePos = file->pos();

		prepared = true;
	}

	void ModelBIN::loadRamThread()
	{
		file->seek(filePos);

		pos.resize(vertexNumber);
		color.resize(3 * vertexNumber);
		if (hasIntensity()) intensity.resize(vertexNumber);

		file->read((char*)pos.data(), vertexNumber * sizeof(glm::vec3));
		file->read((char*)color.data(), 3 * vertexNumber * sizeof(unsigned char));
		if (hasIntensity()) file->read((char*)intensity.data(), vertexNumber * sizeof(unsigned char));
	}

	void ModelBIN::render(QOpenGLFunctions* f)
	{
		shader->setUniformValue("pointSize", getPointSize());

        if (posBuffer.isCreated())
        {
            posBuffer.bind();
            shader->enableAttributeArray("in_vertex");
            shader->setAttributeBuffer("in_vertex", GL_FLOAT, 0, 3);
            posBuffer.release();
        }

        if (colorBuffer.isCreated())
        {
            colorBuffer.bind();
            shader->enableAttributeArray("in_color");
            shader->setAttributeArray("in_color", GL_UNSIGNED_BYTE, 0, 3);
            colorBuffer.release();
        }

        if (hasIntensity())
        {
            intensityBuffer.bind();
            shader->enableAttributeArray("in_intensity");
            shader->setAttributeArray("in_intensity", GL_UNSIGNED_BYTE, 0, 1);
            intensityBuffer.release();
        }

        f->glDrawArrays(primitives, 0, vertexNumber);

        shader->disableAttributeArray("in_vertex");
        shader->disableAttributeArray("in_color");
		shader->disableAttributeArray("in_intensity");
	}

}