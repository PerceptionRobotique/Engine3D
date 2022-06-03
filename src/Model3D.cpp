#include "Model3D.h"

namespace MIS
{

    unsigned int Model3D::model3DNumber(0);
    QOpenGLBuffer Model3D::boxIndexBuffer(QOpenGLBuffer::IndexBuffer);
    QVector<unsigned int> Model3D::boxIndex{ 0, 1, 1, 2, 2, 3, 3, 0, 0, 4, 1, 5, 2, 6, 3, 7, 4, 5, 5, 6, 6, 7, 7, 4 };
    QMutex Model3D::vertexNumberMutex;
    unsigned long long Model3D::vertexOnRAM(0);
    unsigned long long Model3D::vertexOnVRAM(0);
    QVector<unsigned long long> Model3D::vertexAddedToRAM;
    QVector<unsigned long long> Model3D::vertexAddedToVRAM;

    Model3D::Model3D(QString _fileName)
        : vertexNumber(0)
        , prepared(false)
        , showIntensity(false)
        , visible(true)
        , boxVisible(false)
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
        model3DNumber++;

        connect(this, SIGNAL(objectChanged()), this, SIGNAL(modelChanged()));

        if (!fileName.isEmpty())
        {
            file = new QFile(fileName);
            file->open(QFile::ReadOnly);
            name = fileInfo.fileName();
#ifndef ANDROID
            settings = new QSettings(fileInfo.path() + '/' + fileInfo.fileName().remove(fileInfo.suffix()) + "ini", QSettings::IniFormat);
#else
            name = name.split("%2F").last();
            settings = new QSettings;
            settings->beginGroup(name);
#endif
            setwMo(stringToMat4(settings->value("wMo").toString()));

            //POSES
            settings->beginGroup("poses");
            for (const QString& key : settings->childKeys())
                 storedPoses[key] = stringToMat4(settings->value(key).toString());
            settings->endGroup();
        }
    }

    Model3D::~Model3D()
    {
        if (file != nullptr)
        {
            file->close();
            delete file;
            file = nullptr;
        }

        if (onRAM) unloadRAM();
        if (onVRAM) unloadVRAM();
        if (boxOnRAM) unloadBoxRAM();
        if (boxOnVRAM) unloadBoxVRAM();

        model3DNumber--;
        if (model3DNumber == 0) boxIndexBuffer.destroy();

        if (settings)
        {
            settings->setValue("wMo", QString::fromStdString(to_string(getwMo())));
            settings->sync();
            delete settings;
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

    void Model3D::addVertexOnRAM()
    {
        vertexNumberMutex.lock();
        vertexOnRAM += vertexNumber;
        vertexNumberMutex.unlock();
        emit vertexOnRAMChanged();
    }

    void Model3D::removeVertexOnRAM()
    {
        vertexNumberMutex.lock();
        vertexOnRAM -= vertexNumber;
        vertexNumberMutex.unlock();
        emit vertexOnRAMChanged();
    }

    void Model3D::addVertexOnVRAM()
    {
        vertexNumberMutex.lock();
        vertexOnVRAM += vertexNumber;
        vertexNumberMutex.unlock();
        emit vertexOnVRAMChanged();
    }

    void Model3D::removeVertexOnVRAM()
    {
        vertexNumberMutex.lock();
        vertexOnVRAM -= vertexNumber;
        vertexNumberMutex.unlock();
        emit vertexOnVRAMChanged();
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

    bool Model3D::isLiveLoading() const
    {
        return liveLoading;
    }

    bool Model3D::isOnRAM() const
    {
        return onRAM;
    }

    bool Model3D::isOnVRAM() const
    {
        return onVRAM;
    }

    bool Model3D::isGlobalColorEnabled() const
    {
        return globalColorEnabled;
    }

    QColor Model3D::getGlobalColor() const
    {
        return globalColor;
    }

    QHash<QString, mat4> Model3D::getStoredPoses() const
    {
        return storedPoses;
    }

    mat4 Model3D::getStoredPose(const QString& name) const
    {
        return storedPoses[name];
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

    void Model3D::setAABB(AABB _aabb)
    {
        aabb = _aabb;
        unloadBoxVRAM();
        loadBoxRAM();
        emit modelChanged();
    }

    void Model3D::loadRAM(bool force)
    {
        if (force) vertexLoader.lock();
        if (vertexLoader.tryLock() || force)
        {
            if (!onRAM)
            {
                loadRamThread();
                addVertexOnRAM();
                onRAM = true;
                emit modelLoaded();
            }
            vertexLoader.unlock();
        }
        else emit modelLoadingDelayed();
    }

    void Model3D::unloadRAM(bool force)
    {
        if (force) vertexLoader.lock();
        if (vertexLoader.tryLock() || force)
        {
            if (onRAM)
            {
                pos.clear();
                pos.squeeze();
                color.clear();
                color.squeeze();
                intensity.clear();
                intensity.squeeze();

                removeVertexOnRAM();
                onRAM = false;
                emit modelUnloaded();
            }
            vertexLoader.unlock();
        }
        else emit modelLoadingDelayed();
    }

    void Model3D::loadVRAM(bool force)
    {
        if (force) vertexLoader.lock();
        if (vertexLoader.tryLock() || force)
        {
            if (onRAM && !onVRAM)
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
                addVertexOnVRAM();
                onVRAM = true;
            }
            vertexLoader.unlock();
        }
        else emit modelLoadingDelayed();
    }

    void Model3D::unloadVRAM(bool force)
    {
        if (force) vertexLoader.lock();
        if (vertexLoader.tryLock() || force)
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
                removeVertexOnVRAM();
                onVRAM = false;
            }
            vertexLoader.unlock();
        }
        else emit modelLoadingDelayed();
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
            if (isVisible())
            {
                if (isOnVRAM())
                {
                    mat4 wMo = getwMo();
                    f->glUniformMatrix4fv(f->glGetUniformLocation(shader->programId(), "model"), 1, GL_FALSE, value_ptr(wMo));

                    shader->setUniformValue("customColor", isGlobalColorEnabled());
                    shader->setUniformValue("R", getGlobalColor().redF());
                    shader->setUniformValue("G", getGlobalColor().greenF());
                    shader->setUniformValue("B", getGlobalColor().blueF());
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
        if (isPrepared())
        {
            if (isBoxVisible())
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

                    if (!boxIndexBuffer.isCreated())
                    {
                        boxIndexBuffer.create();
                        boxIndexBuffer.bind();
                        boxIndexBuffer.allocate(boxIndex.constData(), boxIndex.count() * (int)sizeof(unsigned int));
                        boxIndexBuffer.release();
                    }
                    boxIndexBuffer.bind();
                    f->glDrawElements(GL_LINES, boxIndex.count(), GL_UNSIGNED_INT, 0);
                    boxIndexBuffer.release();
                    shader->disableAttributeArray("pos");

                    drawn = true;
                }
            }
        }
        return drawn;
    }

    void Model3D::setwMo(mat4 wMo)
    {
        setPose(wMo);
    }

    void Model3D::setGlobalColorEnabled(bool enabled)
    {
        globalColorEnabled = enabled;
        emit modelChanged();
    }

    void Model3D::setGlobalColor(QColor color)
    {
        globalColor = color;
        emit modelChanged();
    }

    bool Model3D::addStoredPose(const QString& name, mat4 pose)
    {
        settings->beginGroup("poses");
        bool ok = true;
        for (const QString& key : settings->childKeys())
            ok &= key.toLower() != name.toLower();
        if (ok)
        {
            settings->setValue(name, QString::fromStdString(glm::to_string(pose)));
            storedPoses[name] = pose;
        }
        settings->endGroup();
        return ok;
    }

    void Model3D::removeStoredPose(const QString& name)
    {
        storedPoses.remove(name);
        settings->beginGroup("poses");
        settings->remove(name);
        settings->endGroup();
    }

    void Model3D::unloadRAMthread()
    {
        pos.clear();
        pos.squeeze();
        color.clear();
        color.squeeze();
        intensity.clear();
        intensity.squeeze();

        removeVertexOnRAM();
        onRAM = false;
        vertexLoader.unlock();
    }

}
