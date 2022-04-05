#include "Model3D.h"

unsigned int Model3D::model3DNumber(0);
QOpenGLBuffer Model3D::boxIndexBuffer(QOpenGLBuffer::IndexBuffer);
QVector<unsigned int> Model3D::boxIndex{ 0, 1, 1, 2, 2, 3, 3, 0, 0, 4, 1, 5, 2, 6, 3, 7, 4, 5, 5, 6, 6, 7, 7, 4 };
QMutex Model3D::vertexNumberMutex;
unsigned long long Model3D::vertexOnRAM(0);
unsigned long long Model3D::vertexOnVRAM(0);

Model3D::Model3D(QString _fileName)
    : vertexNumber(0)
    , prepared(false)
    , showIntensity(false)
    , visible(true)
    , boxVisible(false)
    , onScreen(false)
    , boxOnRAM(false)
    , boxOnVRAM(false)
    , onVRAM(false)
    , box(8, glm::vec3(0, 0, 0))
    , fileName(_fileName)
    , file(nullptr)
    , fileInfo(_fileName)
    , settings(nullptr)
    , m_hasIntensity(false)
    , onRAM(false)
    , globalColorEnabled(false)
    , globalColor(255, 255, 255, 255)
    , boxColor(0, 255, 0, 255)
    , boxBuffer(QOpenGLBuffer::VertexBuffer)
    , posBuffer(QOpenGLBuffer::VertexBuffer)
    , colorBuffer(QOpenGLBuffer::VertexBuffer)
    , intensityBuffer(QOpenGLBuffer::VertexBuffer)
{
    static QMutex model3DCreateMutex;
    model3DCreateMutex.lock();
    if (model3DNumber == 0)
    {
        boxIndexBuffer.create();
        boxIndexBuffer.bind();
        boxIndexBuffer.allocate(boxIndex.constData(), boxIndex.count() * (int)sizeof(unsigned int));
        boxIndexBuffer.release();
    }
    model3DNumber++;
    model3DCreateMutex.unlock();

    connect(this, SIGNAL(objectChanged()), this, SIGNAL(modelChanged()));

    if (!fileName.isEmpty())
    {
        file = new QFile(fileName);
        file->open(QFile::ReadOnly);
        name = fileInfo.fileName();
        settings = new QSettings(fileInfo.path() + '/' + fileInfo.fileName().remove(fileInfo.suffix()) + "ini", QSettings::IniFormat);

        mat4 wMo(1.0);
        QVector<QVariant> v_wMo;
        QVector<QVariant> def(16);
        for (unsigned int i = 0; i < 4; i++)
        {
            for (unsigned int j = 0; j < 4; j++)
            {
                def[i * 4 + j] = wMo[i][j];
            }
        }
        v_wMo = settings->value("wMo", def).toList();
        for (unsigned int i = 0; i < 4; i++)
        {
            for (unsigned int j = 0; j < 4; j++)
            {
                wMo[i][j] = v_wMo[i * 4 + j].toFloat();
            }
        }
        setwMo(wMo);
    }
}

Model3D::~Model3D()
{
    ramLoader.cancel();

    if (file != nullptr)
    {
        file->close();
        delete file;
        file = nullptr;
    }

    if (onRAM) unloadRAM(true);
    if (onVRAM) unloadVRAM();
    if (boxOnRAM) unloadBoxRAM();
    if (boxOnVRAM) unloadBoxVRAM();

    model3DNumber--;
    if (model3DNumber == 0)
        boxIndexBuffer.destroy();

    if (settings)
    {
        mat4 wMo = getwMo();
        QVector<QVariant> v_wMo;
        for (unsigned int i = 0; i < 4; i++)
        {
            for (unsigned int j = 0; j < 4; j++)
            {
                v_wMo.append(wMo[i][j]);
            }
        }
        settings->setValue("wMo", v_wMo);
        settings->sync();
    }
    emit modelDestroyed();
}

unsigned long long Model3D::getVertexOnRAM()
{
    unsigned long long value;
    vertexNumberMutex.lock();
    value = vertexOnRAM;
    vertexNumberMutex.unlock();
    return value;
}

unsigned long long Model3D::getVertexOnVRAM()
{
    unsigned long long value;
    vertexNumberMutex.lock();
    value = vertexOnVRAM;
    vertexNumberMutex.unlock();
    return value;
}

void Model3D::setVertexOnRAM(unsigned long long value)
{
    vertexNumberMutex.lock();
    vertexOnRAM = value;
    emit vertexOnRAMChanged(vertexOnRAM);
    vertexNumberMutex.unlock();
}

void Model3D::setVertexOnVRAM(unsigned long long value)
{
    vertexNumberMutex.lock();
    vertexOnVRAM = value;
    emit vertexOnVRAMChanged(vertexOnVRAM);
    vertexNumberMutex.unlock();
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

QVector<glm::vec3> Model3D::getBox() const
{
    return box;
}

unsigned long long Model3D::getVertexNumber() const
{
    return vertexNumber;
}

QVector<glm::vec3>& Model3D::getPos()
{
    return pos;
}

QVector<unsigned char>& Model3D::getColor()
{
    return color;
}

QVector<unsigned char>& Model3D::getIntensity()
{
    return intensity;
}

bool Model3D::hasIntensity() const
{
    return m_hasIntensity;
}

bool Model3D::getShowIntensity() const
{
    return showIntensity;
}

bool Model3D::isVisible() const
{
    return visible;
}

bool Model3D::isBoxVisible() const
{
    return boxVisible;
}

bool Model3D::isPrepared() const
{
    return prepared;
}

bool Model3D::isOnScreen() const
{
    return onScreen;
}

bool Model3D::isOnRAM() const
{
    return onRAM;
}

bool Model3D::isOnVRAM() const
{
    return onVRAM;
}

void Model3D::waitRAMloading()
{
    ramLoader.waitForFinished();
}

bool Model3D::isGlobalColorEnabled() const
{
    return globalColorEnabled;
}

QColor Model3D::getGlobalColor() const
{
    return globalColor;
}

void Model3D::setVisible(bool _visible)
{
    visible = _visible;
    emit modelChanged();
}

void Model3D::setBoxVisible(bool _boxVisible)
{
    boxVisible = _boxVisible;
    emit modelChanged();
}

void Model3D::setShowIntensity(bool _showIntensity)
{
    showIntensity = _showIntensity;
    emit modelChanged();
}

void Model3D::setOnScreen(bool _onScreen, bool force)
{
    if (onScreen != _onScreen)
    {
        onScreen = _onScreen;
        emit modelChanged();
    }
    if (onScreen) loadRAM(force);
    else if (liveLoading) unloadRAM(force);
}

void Model3D::setAABB(AABB _aabb)
{
    aabb = _aabb;
    unloadBoxVRAM();
    loadBoxRAM();
    emit modelChanged();
}

void Model3D::loadRAM(bool force)
{
    if (prepared && !onRAM && !ramLoader.isRunning())
    {
        if (force) vertexLoader.lock();
        if(force ? true : vertexLoader.tryLock()) ramLoader = QtConcurrent::run(&Model3D::loadRAMthread, this);
        else emit modelLoadingDelayed();
    }
}

void Model3D::unloadRAM(bool force)
{
    if (onRAM)
    {
        if (force) vertexLoader.lock();
        if (vertexLoader.tryLock() || force)
        {
            pos.clear();
            pos.squeeze();
            color.clear();
            color.squeeze();
            intensity.clear();
            intensity.squeeze();

            setVertexOnRAM(getVertexOnRAM() - vertexNumber);
            onRAM = false;
            vertexLoader.unlock();
        }
        else emit modelLoadingDelayed();
    }
}

void Model3D::loadVRAM()
{
    if (onRAM && !onVRAM)
    {
        if (vertexLoader.tryLock())
        {
            if (!posBuffer.isCreated()) posBuffer.create();
            posBuffer.bind();
            QOpenGLContext::currentContext()->functions()->glBufferData(GL_ARRAY_BUFFER, vertexNumber * sizeof(glm::vec3), pos.constData(), GL_STATIC_DRAW);
            posBuffer.release();

            if (!colorBuffer.isCreated()) colorBuffer.create();
            colorBuffer.bind();
            QOpenGLContext::currentContext()->functions()->glBufferData(GL_ARRAY_BUFFER, 3 * vertexNumber * sizeof(unsigned char), color.constData(), GL_STATIC_DRAW);
            colorBuffer.release();

            if (hasIntensity())
            {
                if (!intensityBuffer.isCreated()) intensityBuffer.create();
                intensityBuffer.bind();
                QOpenGLContext::currentContext()->functions()->glBufferData(GL_ARRAY_BUFFER, vertexNumber * sizeof(unsigned char), intensity.constData(), GL_STATIC_DRAW);
                intensityBuffer.release();
            }
            setVertexOnVRAM(getVertexOnVRAM() + vertexNumber);
            onVRAM = true;
            vertexLoader.unlock();
        }
        else emit modelLoadingDelayed();
    }
}

void Model3D::unloadVRAM()
{
    if (onVRAM)
    {
        vertexLoader.lock();
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
        onVRAM = false;
        vertexLoader.unlock();
    }
}

void Model3D::loadBoxRAM()
{
    boxLoaderMutex.lock();
    if (!boxOnRAM)
    {
        box.clear();
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

        boxOnRAM = true;
    }
    boxLoaderMutex.unlock();
}

void Model3D::unloadBoxRAM()
{
    if (boxOnRAM)
    {
        boxLoaderMutex.lock();
        box.clear();
        box.squeeze();
        boxOnRAM = false;
        boxLoaderMutex.unlock();
    }
}

void Model3D::loadBoxVRAM()
{
    if (boxOnRAM && !boxOnVRAM)
    {
        if (!boxBuffer.isCreated()) boxBuffer.create();
        boxBuffer.bind();
        boxBuffer.allocate(box.constData(), box.count() * sizeof(glm::vec3));
        boxBuffer.release();
        boxOnVRAM = true;
    }
}

void Model3D::unloadBoxVRAM()
{
    if (boxOnVRAM)
    {
        boxBuffer.bind();
        boxBuffer.allocate(0);
        boxBuffer.release();
        boxOnVRAM = false;
    }
}

bool Model3D::draw(QOpenGLShaderProgram* shader)
{
    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
    bool drawn = false;
    if (isPrepared())
    {
        if (isOnScreen() && isVisible())
        {
            if (isOnVRAM())
            {
                mat4 wMo = getwMo();
                f->glUniformMatrix4fv(f->glGetUniformLocation(shader->programId(), "model"), 1, GL_FALSE, value_ptr(wMo));

                shader->setUniformValue("customColor", globalColorEnabled);
                shader->setUniformValue("R", globalColor.redF());
                shader->setUniformValue("G", globalColor.greenF());
                shader->setUniformValue("B", globalColor.blueF());
                shader->setUniformValue("showIntensity", getShowIntensity());

                posBuffer.bind();
                shader->enableAttributeArray("in_vertex");
                shader->setAttributeBuffer("in_vertex", GL_FLOAT, 0, 3);
                posBuffer.release();

                colorBuffer.bind();
                shader->enableAttributeArray("in_color");
                shader->setAttributeArray("in_color", GL_UNSIGNED_BYTE, 0, 3);
                colorBuffer.release();

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
                if (hasIntensity())
                    shader->disableAttributeArray("in_intensity");

                drawn = true;
            }
            else emit modelLoadingDelayed();
        }
    }
    return drawn;
}

bool Model3D::drawBox(QOpenGLShaderProgram* shader)
{
    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
    bool drawn = false;
    if (prepared && onScreen)
    {
        if (boxVisible)
        {
            QColor boxColor = Qt::green;

            loadBoxRAM();
            loadBoxVRAM();
            if (boxOnVRAM)
            {
                mat4 wMo = getwMo();
                glm::vec3 color = glm::vec3(boxColor.redF(), boxColor.greenF(), boxColor.blueF());
                f->glUniformMatrix4fv(f->glGetUniformLocation(shader->programId(), "model"), 1, GL_FALSE, value_ptr(wMo));
                f->glUniform3fv(f->glGetUniformLocation(shader->programId(), "in_color"), 1, value_ptr(color));

                boxBuffer.bind();
                shader->enableAttributeArray("pos");
                shader->setAttributeBuffer("pos", GL_FLOAT, 0, 3);
                boxBuffer.release();

                boxIndexBuffer.bind();
                f->glDrawElements(GL_LINES, boxIndex.count(), GL_UNSIGNED_INT, 0);
                boxIndexBuffer.release();
                shader->disableAttributeArray("pos");

                drawn = true;
            }
        }
    }
    if (!onScreen) unloadBoxVRAM();
    return drawn;
}

void Model3D::setwMo(mat4 wMo)
{
    setPose(wMo);
}

void Model3D::endRAMloading()
{
    onRAM = true;
    vertexLoader.unlock();
    setVertexOnRAM(getVertexOnRAM() + vertexNumber);
    emit modelLoaded();
}