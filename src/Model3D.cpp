#include "Model3D.h"

unsigned int Model3D::model3DNumber(0);
QOpenGLBuffer Model3D::boxIndexBuffer(QOpenGLBuffer::IndexBuffer);
QVector<unsigned int> Model3D::boxIndex{ 0, 1, 1, 2, 2, 3, 3, 0, 0, 4, 1, 5, 2, 6, 3, 7, 4, 5, 5, 6, 6, 7, 7, 4 };
QMutex Model3D::vertexMutex;
unsigned long long Model3D::vertexOnRAM(0);
unsigned long long Model3D::vertexOnVRAM(0);

Model3D::Model3D(QString _fileName)
    : vertexNumber(0)
    , prepared(false)
    , showIntensity(false)
    , onVRAM(false)
    , fileName(_fileName)
    , file(_fileName)
    , fileInfo(_fileName)
    , m_hasIntensity(false)
    , onRAM(false)
    , globalColorEnabled(false)
    , globalColor(255, 255, 255, 255)
    , posBuffer(QOpenGLBuffer::VertexBuffer)
    , colorBuffer(QOpenGLBuffer::VertexBuffer)
    , intensityBuffer(QOpenGLBuffer::VertexBuffer)
{
    static QMutex model3DCreateMutex;
    model3DCreateMutex.lock();
    if (!model3DNumber == 0)
    {
        boxIndexBuffer.create();
        boxIndexBuffer.bind();
        boxIndexBuffer.allocate(boxIndex.constData(), boxIndex.count() * (int)sizeof(unsigned int));
        boxIndexBuffer.release();
    }
    model3DNumber++;
    model3DCreateMutex.unlock();

    if (!fileName.isEmpty())
    {
        file.open(QFile::ReadOnly);
        name = fileInfo.fileName();
    }
}

unsigned long long Model3D::getVertexOnRAM()
{
    unsigned long long value;
    vertexMutex.lock();
    value = vertexOnRAM;
    vertexMutex.unlock();
    return value;
}

unsigned long long Model3D::getVertexOnVRAM()
{
    unsigned long long value;
    vertexMutex.lock();
    value = vertexOnVRAM;
    vertexMutex.unlock();
    return value;
}

void Model3D::setVertexOnRAM(unsigned long long value)
{
    vertexMutex.lock();
    vertexOnRAM = value;
    vertexMutex.unlock();
}

void Model3D::setVertexOnVRAM(unsigned long long value)
{
    vertexMutex.lock();
    vertexOnVRAM = value;
    vertexMutex.unlock();
}

Model3D::~Model3D()
{
    file.close();
    if (onRAM) unloadRAM();
    if (onVRAM) unloadVRAM();
    model3DNumber--;
    if (model3DNumber == 0) boxIndexBuffer.destroy();
}

QString Model3D::getName() const
{
    return name;
}

Model3D::Primitives Model3D::getPrimitives() const
{
    return primitives;
}

mat4 Model3D::getwMo() const
{
    return getPose();
}

Model3D::AABB Model3D::getAABB() const
{
    return aabb;
}

QVector<glm::vec3> Model3D::getBox()
{
    if (box.isEmpty()) loadBoxRAM();
    return box;
}

bool Model3D::hasIntensity() const
{
    return m_hasIntensity;
}

bool Model3D::getShowIntensity() const
{
    return showIntensity;
}

unsigned long long Model3D::getVertexNumber() const
{
    return vertexNumber;
}

bool Model3D::isPrepared() const
{
    return prepared;
}

bool Model3D::isOnRAM() const
{
    return onRAM;
}

bool Model3D::isOnVRAM() const
{
    return onVRAM;
}

void Model3D::unloadRAM()
{
    if (onRAM)
    {
        ramMutex.lock();
        pos.clear();
        pos.squeeze();
        color.clear();
        color.squeeze();
        intensity.clear();
        intensity.squeeze();

        setVertexOnRAM(getVertexOnRAM() - vertexNumber);
        ramMutex.unlock();
    }
}

void Model3D::loadVRAM()
{
    if (onRAM && !onVRAM)
    {
        if (!posBuffer.isCreated()) posBuffer.create();
        posBuffer.bind();
        QOpenGLContext::currentContext()->functions()->glBufferData(GL_ARRAY_BUFFER, 3 * pos.count() * sizeof(float), pos.constData(), GL_STATIC_DRAW);
        posBuffer.release();

        if (!colorBuffer.isCreated()) colorBuffer.create();
        colorBuffer.bind();
        QOpenGLContext::currentContext()->functions()->glBufferData(GL_ARRAY_BUFFER, 3 * color.count() * sizeof(float), color.constData(), GL_STATIC_DRAW);
        colorBuffer.release();

        if (hasIntensity())
        {
            if (!intensityBuffer.isCreated()) intensityBuffer.create();
            intensityBuffer.bind();
            QOpenGLContext::currentContext()->functions()->glBufferData(GL_ARRAY_BUFFER, intensity.count() * sizeof(float), intensity.constData(), GL_STATIC_DRAW);
            intensityBuffer.release();
        }
        setVertexOnVRAM(getVertexOnVRAM() + vertexNumber);
        onVRAM = true;
    }
}

void Model3D::unloadVRAM()
{
    if (onVRAM)
    {
        posBuffer.bind();
        posBuffer.allocate(0);
        posBuffer.release();

        colorBuffer.bind();
        colorBuffer.allocate(0);
        colorBuffer.release();

        if (hasIntensity())
        {
            intensityBuffer.bind();
            intensityBuffer.allocate(0);
            intensityBuffer.release();
        }
        setVertexOnVRAM(getVertexOnVRAM() - vertexNumber);
    }
}

void Model3D::loadBoxRAM()
{
    if (box.isEmpty())
    {
        box.append(vec3(
            aabb.min.x,
            aabb.min.y,
            aabb.min.z
        ));

        box.append(vec3(
            aabb.max.x,
            aabb.min.y,
            aabb.min.z
        ));

        box.append(vec3(
            aabb.max.x,
            aabb.min.y,
            aabb.max.z
        ));

        box.append(vec3(
            aabb.min.x,
            aabb.min.y,
            aabb.max.z
        ));

        box.append(vec3(
            aabb.min.x,
            aabb.max.y,
            aabb.min.z
        ));

        box.append(vec3(
            aabb.max.x,
            aabb.max.y,
            aabb.min.z
        ));

        box.append(vec3(
            aabb.max.x,
            aabb.max.y,
            aabb.max.z
        ));

        box.append(vec3(
            aabb.min.x,
            aabb.max.y,
            aabb.max.z
        ));
    }
}

void Model3D::unloadBoxRAM()
{
    box.clear();
    box.squeeze();
}

void Model3D::loadBoxVRAM()
{
    if (!boxBuffer.isCreated()) boxBuffer.create();
    boxBuffer.bind();
    boxBuffer.allocate(box.constData(), box.count() * (int)sizeof(float));
    boxBuffer.release();
}

void Model3D::unloadBoxVRAM()
{
    boxBuffer.bind();
    boxBuffer.allocate(0);
    boxBuffer.release();
}

void Model3D::draw(QOpenGLShaderProgram* shader)
{
    if (onVRAM)
    {
        shader->setUniformValue("customColor", globalColorEnabled);
        shader->setUniformValue("R", globalColor.redF());
        shader->setUniformValue("G", globalColor.greenF());
        shader->setUniformValue("B", globalColor.blueF());

        mat4 wMo = getwMo();
        QOpenGLContext::currentContext()->functions()->glUniformMatrix4fv(QOpenGLContext::currentContext()->functions()->glGetUniformLocation(shader->programId(), "model"), 1, GL_FALSE, value_ptr(wMo));
        shader->setUniformValue("showIntensity", getShowIntensity());

        posBuffer.bind();
        shader->enableAttributeArray("in_vertex");
        shader->setAttributeBuffer("in_vertex", GL_FLOAT, 0, 3);
        posBuffer.release();

        colorBuffer.bind();
        shader->enableAttributeArray("in_color");
        shader->setAttributeArray("in_color", GL_FLOAT, 0, 3);
        colorBuffer.release();

        if (hasIntensity())
        {
            intensityBuffer.bind();
            shader->enableAttributeArray("in_intensity");
            shader->setAttributeArray("in_intensity", GL_FLOAT, 0, 1);
            intensityBuffer.release();
        }

        QOpenGLContext::currentContext()->functions()->glDrawArrays(primitives, 0, vertexNumber);

        shader->disableAttributeArray("in_vertex");
        shader->disableAttributeArray("in_color");
        if (m_hasIntensity)
            shader->disableAttributeArray("in_intensity");
    }
}
