#include "ModelCustom.h"

namespace MIS
{

	ModelCustom::ModelCustom(Primitives primitives, bool liveLoading, QOpenGLShaderProgram* shader, QOpenGLShaderProgram* boxShader)
		: Model3D("", shader, boxShader)
	{
		this->primitives = primitives;
		this->liveLoading = liveLoading;
	}

	void ModelCustom::loadRamThread()
	{
        pos = p;
        color = c;
        intensity = i;
	}

    QVector<vec3>& ModelCustom::getPoints()
    {
        return p;
    }

    void ModelCustom::addVertex(const QVector<vec3>& pos, const QVector<unsigned char>& color, const QVector<unsigned char>& intensity)
    {
        prepared = !p.isEmpty() || !pos.isEmpty();

        clearMemory();

        p.append(pos);
        c.append(color);
        if (!intensity.isEmpty())
        {
            i.append(intensity);
            m_hasIntensity = true;
        }

        vertexNumber = p.count();

        updateAABB();

        emit modelChanged();
    }

    void ModelCustom::addVertex(const vec3& pos, const QVector<unsigned char>& color, const QVector<unsigned char>& intensity)
    {
        clearMemory();

        p.append(pos);
        c.append(color);
        if (!intensity.isEmpty())
        {
            i.append(intensity);
            m_hasIntensity = true;
        }

        prepared = !p.isEmpty();

        vertexNumber = p.count();

        updateAABB();

        emit modelChanged();
    }

    void ModelCustom::setVertex(const QVector<vec3>& pos, const QVector<unsigned char>& color, const QVector<unsigned char>& intensity)
    {
        prepared = !pos.isEmpty();

        clearMemory();

        p = pos;
        c = color;
        if (!intensity.isEmpty())
        {
            i = intensity;
            m_hasIntensity = true;
        }

        vertexNumber = p.count();

        updateAABB();

        emit modelChanged();
    }

    void ModelCustom::removeVertex(unsigned long long index)
    {
        clearMemory();

        p.remove(index);
        c.remove(3 * index, 3);
        if (!i.isEmpty())
            i.remove(index);

        updateAABB();
        emit modelChanged();
        emit modelRenderChanged();
    }

    void ModelCustom::clear()
    {
        prepared = false;

        clearMemory();

        p.clear();
        p.squeeze();
        c.clear();
        c.squeeze();
        i.clear();
        i.squeeze();

        updateAABB();

        emit modelChanged();
    }

	void ModelCustom::render(QOpenGLFunctions* f)
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

    void ModelCustom::clearMemory()
    {
        posBuffer.destroy();
        colorBuffer.destroy();
        intensityBuffer.destroy();
        unloadVRAM();
        unloadRAM();
    }

    void ModelCustom::updateAABB()
    {
        AABB aabb;

        if (!p.isEmpty())
        {
            aabb.min = p[0];
            aabb.max = p[0];
            aabb.gravity = vec3(0, 0, 0);

            for (const vec3& v : p)
            {
                aabb.min.x = qMin(aabb.min.x, v.x);
                aabb.min.y = qMin(aabb.min.y, v.y);
                aabb.min.z = qMin(aabb.min.z, v.z);

                aabb.max.x = qMax(aabb.max.x, v.x);
                aabb.max.y = qMax(aabb.max.y, v.y);
                aabb.max.z = qMax(aabb.max.z, v.z);

                aabb.gravity += v;
            }

            aabb.gravity /= p.count();
        }

        aabb.updateCenter();
        setAABB(aabb);
    }

}