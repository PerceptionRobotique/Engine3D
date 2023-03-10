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

    Model3D::Model3D(QString _fileName, QOpenGLShaderProgram* shader, QOpenGLShaderProgram* boxShader)
        : vertexNumber(0)
        , scale(1.0f)
        , pointSizeEnabled(false)
        , pointSize(1.0f)
        , axisModifier(1, 1, 1)
        , prepared(false)
        , liveLoading(true)
        , showIntensity(false)
        , visible(true)
        , boxVisible(false)
        , boxOnRAM(false)
        , boxOnVRAM(false)
        , onVRAM(false)
        , opacityEnabled(false)
        , opacity(1.0f)
        , blendFunction(BLEND_1)
        , box(8, glm::vec3(0, 0, 0))
        , shader(shader)
        , boxShader(boxShader)
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

        connect(this, SIGNAL(objectMoved()), this, SIGNAL(modelMoved()));

        if (!fileName.isEmpty())
        {
            file = new QFile(fileName);
            file->open(QFile::ReadOnly);
            name = fileInfo.fileName();
#ifndef ANDROID
            settings = new QSettings(fileInfo.path() + '/' + fileInfo.fileName().split(".")[0] + ".ini", QSettings::IniFormat);
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

        if (posBuffer.isCreated()) posBuffer.destroy();
        if (colorBuffer.isCreated()) colorBuffer.destroy();
        if (intensityBuffer.isCreated()) intensityBuffer.destroy();
        for(QVector<QOpenGLBuffer>& pbl : pointBuffer)
            for(QOpenGLBuffer& pb : pbl)
                if(pb.isCreated())
                    pb.destroy();
        for(QVector<QOpenGLBuffer>& nbl : normalBuffer)
            for (QOpenGLBuffer& nb : nbl)
                if (nb.isCreated())
                nb.destroy();
        for(QVector<QOpenGLBuffer>& uvl : uvBuffer)
            for (QOpenGLBuffer& ub : uvl)
                if (ub.isCreated())
                    ub.destroy();

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

    float Model3D::getScale() const
    {
        return scale;
    }

    bool Model3D::isPointSizeEnabled() const
    {
        return pointSizeEnabled;
    }

    float Model3D::getPointSize() const
    {
        return pointSize;
    }

    vec3 Model3D::getAxisModifier() const
    {
        return axisModifier;
    }

    mat4 Model3D::getwMo() const
    {
        return getPose();
    }

    Model3D::AABB Model3D::getAABB() const
    {
        return getScale() * getAxisModifier() * aabb;
    }

    bool Model3D::isPointInAABB(vec3 point) const
    {
        AABB aabb = getwMo() * getAABB();
        return
            point.x >= aabb.min.x &&
            point.y >= aabb.min.y &&
            point.z >= aabb.min.z &&
            point.x <= aabb.max.x &&
            point.y <= aabb.max.y &&
            point.z <= aabb.max.z;
    }

    QVector<glm::vec3> Model3D::getBox() const
    {
        QVector<vec3> b;
        for (vec3 point : box)
            b.append(getScale() * getAxisModifier() * point);
        return b;
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

    glm::vec3 Model3D::getPosAt(unsigned long long index)
    {
        bool wasOnRAM = isOnRAM();
        if (!isOnRAM()) loadRAM(true);
        glm::vec3 res = pos[index];
        if(!wasOnRAM) unloadRAM(true);
        return res;
    }

    QVector<unsigned char> Model3D::getColorAt(unsigned long long index)
    {
        bool wasOnRAM = isOnRAM();
        if (!isOnRAM()) loadRAM(true);
        QVector<unsigned char> res(3);
        for(unsigned int i = 0 ; i < 3 ; i++)
            res[i] = color[3 * index + i];
        if (!wasOnRAM) unloadRAM(true);
        return res;
    }

    unsigned char Model3D::getIntensityAt(unsigned long long index)
    {
        if (hasIntensity())
        {
            bool wasOnRAM = isOnRAM();
            if(!isOnRAM()) loadRAM(true);
            unsigned char res = intensity[index];
            if (!wasOnRAM) unloadRAM(true);
            return res;
        }
        else return 0;
    }

    bool Model3D::isOpacityEnabled() const
    {
        return opacityEnabled;
    }

    float Model3D::getOpacity() const
    {
        return opacity;
    }

    Model3D::BlendFunction Model3D::getBlendFunction() const
    {
        return blendFunction;
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
        emit modelRenderChanged();
    }

    void Model3D::setBoxVisible(bool _boxVisible)
    {
        boxVisible = _boxVisible;
        emit modelRenderChanged();
    }

    void Model3D::setShowIntensity(bool _showIntensity)
    {
        showIntensity = _showIntensity;
        emit modelRenderChanged();
    }

    void Model3D::setAABB(AABB _aabb)
    {
        aabb = _aabb;
        unloadBoxVRAM();
        unloadBoxRAM();
        loadBoxRAM();
        emit modelMoved();
    }

    void Model3D::setShader(QOpenGLShaderProgram* shader)
    {
        this->shader = shader;
    }

    void Model3D::setBoxShader(QOpenGLShaderProgram* boxShader)
    {
        this->boxShader = boxShader;
    }

    void Model3D::loadRAM(bool force)
    {
        if (isPrepared())
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
    }

    void Model3D::unloadRAM(bool force)
    {
        if (force) vertexLoader.lock();
        if (vertexLoader.tryLock() || force)
        {
            if (onRAM)
            {
                onRAM = false;
                pos.clear();
                pos.squeeze();
                color.clear();
                color.squeeze();
                intensity.clear();
                intensity.squeeze();

                point.clear();
                point.squeeze();
                normal.clear();
                normal.squeeze();
                uv.clear();
                uv.squeeze();

                removeVertexOnRAM();
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
                if (!pos.isEmpty())
                {
                    if (!posBuffer.isCreated()) posBuffer.create();
                    posBuffer.bind();
                    QOpenGLContext::currentContext()->functions()->glBufferData(GL_ARRAY_BUFFER, vertexNumber * sizeof(glm::vec3), pos.constData(), GL_STATIC_DRAW);
                    posBuffer.release();
                }

                if (!color.isEmpty())
                {
                    if (!colorBuffer.isCreated()) colorBuffer.create();
                    colorBuffer.bind();
                    QOpenGLContext::currentContext()->functions()->glBufferData(GL_ARRAY_BUFFER, 3 * vertexNumber * sizeof(unsigned char), color.constData(), GL_STATIC_DRAW);
                    colorBuffer.release();
                }

                if (hasIntensity())
                {
                    if (!intensityBuffer.isCreated()) intensityBuffer.create();
                    intensityBuffer.bind();
                    QOpenGLContext::currentContext()->functions()->glBufferData(GL_ARRAY_BUFFER, vertexNumber * sizeof(unsigned char), intensity.constData(), GL_STATIC_DRAW);
                    intensityBuffer.release();
                }

                if (!point.isEmpty())
                {
                    pointBuffer.resize(point.count());
                    for (unsigned int i = 0; i < point.count(); i++)
                    {
                        pointBuffer[i].resize(point[i].count());
                        for (unsigned int j = 0; j < point[i].count(); j++)
                        {
                            if (!pointBuffer[i][j].isCreated()) pointBuffer[i][j].create();
                            pointBuffer[i][j].bind();
                            QOpenGLContext::currentContext()->functions()->glBufferData(GL_ARRAY_BUFFER, 3 * point[i][j].count() * sizeof(float), point[i][j].constData(), GL_STATIC_DRAW);
                            pointBuffer[i][j].release();
                        }
                    }
                }

                if (!normal.isEmpty())
                {
                    normalBuffer.resize(normal.count());
                    for (unsigned int i = 0; i < normal.count(); i++)
                    {
                        normalBuffer[i].resize(normal[i].count());
                        for (unsigned int j = 0; j < normal[i].count(); j++)
                        {
                            if (!normalBuffer[i][j].isCreated()) normalBuffer[i][j].create();
                            normalBuffer[i][j].bind();
                            QOpenGLContext::currentContext()->functions()->glBufferData(GL_ARRAY_BUFFER, 3 * normal[i][j].count() * sizeof(float), normal[i][j].constData(), GL_STATIC_DRAW);
                            normalBuffer[i][j].release();
                        }
                    }
                }

                if (!uv.isEmpty())
                {
                    uvBuffer.resize(uv.count());
                    for (unsigned int i = 0; i < uv.count(); i++)
                    {
                        uvBuffer[i].resize(uv[i].count());
                        for (unsigned int j = 0; j < uv[i].count(); j++)
                        {
                            if (!uv[i].isEmpty())
                            {
                                if (!uvBuffer[i][j].isCreated()) uvBuffer[i][j].create();
                                uvBuffer[i][j].bind();
                                QOpenGLContext::currentContext()->functions()->glBufferData(GL_ARRAY_BUFFER, 2 * uv[i][j].count() * sizeof(float), uv[i][j].constData(), GL_STATIC_DRAW);
                                uvBuffer[i][j].release();
                            }
                        }
                    }
                }

                for (QString& key : textures.keys())
                {
                    QOpenGLTexture* textureBuffer = new QOpenGLTexture(QOpenGLTexture::Target2D);
                    textureBuffer->create();
                    textureBuffer->setData(textures[key]);
                    texturesBuffers[key] = textureBuffer;
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
                if (posBuffer.isCreated())
                {
                    posBuffer.bind();
                    posBuffer.allocate(0);
                    posBuffer.release();
                }

                if (colorBuffer.isCreated())
                {
                    colorBuffer.bind();
                    colorBuffer.allocate(0);
                    colorBuffer.release();
                }

                if (hasIntensity())
                {
                    intensityBuffer.bind();
                    intensityBuffer.allocate(0);
                    intensityBuffer.release();
                }

                for (QVector<QOpenGLBuffer>& bufferList : pointBuffer)
                {
                    for (QOpenGLBuffer& buffer : bufferList)
                    {
                        if (buffer.isCreated())
                        {
                            buffer.bind();
                            buffer.allocate(0);
                            buffer.release();
                        }
                    }
                }

                for (QVector<QOpenGLBuffer>& bufferList : normalBuffer)
                {
                    for (QOpenGLBuffer& buffer : bufferList)
                    {
                        if (buffer.isCreated())
                        {
                            buffer.bind();
                            buffer.allocate(0);
                            buffer.release();
                        }
                    }
                }

                for (QVector<QOpenGLBuffer>& bufferList : uvBuffer)
                {
                    for (QOpenGLBuffer& buffer : bufferList)
                    {
                        if (buffer.isCreated())
                        {
                            buffer.bind();
                            buffer.allocate(0);
                            buffer.release();
                        }
                    }
                }

                for (QOpenGLTexture* texture : texturesBuffers)
                {
                    texture->destroy();
                    delete texture;
                }
                texturesBuffers.clear();

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
            boxBuffer.destroy();
            boxOnVRAM = false;
        }
    }

    bool Model3D::draw()
    {
        bool drawn = false;
        if (shader)
        {
            if (shader->bind())
            {
                QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
                if (isPrepared())
                {
                    if (isVisible())
                    {
                        if (isOnVRAM())
                        {
                            shader->setUniformValue("scale", getScale());
                            f->glUniformMatrix4fv(f->glGetUniformLocation(shader->programId(), "wMo"), 1, GL_FALSE, value_ptr(getwMo()));
                            f->glUniformMatrix4fv(f->glGetUniformLocation(shader->programId(), "oMw"), 1, GL_FALSE, value_ptr(inverse(getwMo())));

                            shader->setUniformValue("customColor", isGlobalColorEnabled());
                            shader->setUniformValue("R", getGlobalColor().redF());
                            shader->setUniformValue("G", getGlobalColor().greenF());
                            shader->setUniformValue("B", getGlobalColor().blueF());
                            shader->setUniformValue("showIntensity", getShowIntensity());
                            if (isOpacityEnabled())
                            {
                                f->glEnable(GL_BLEND);
                                if (getBlendFunction() == BLEND_1) f->glEnable(GL_DEPTH_TEST);
                                else if (getBlendFunction() == BLEND_2) f->glDisable(GL_DEPTH_TEST);
                                f->glBlendFunc(GL_SRC_ALPHA, getBlendFunction());
                                shader->setUniformValue("opacity", getOpacity());
                            }
                            else
                            {
                                f->glDisable(GL_BLEND);
                                f->glEnable(GL_DEPTH_TEST);
                                f->glBlendFunc(GL_ONE, GL_ZERO);
                                shader->setUniformValue("opacity", 1.0f);
                            }
                            if (isPointSizeEnabled())
                            {
#ifndef ANDROID
                                f->glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
#endif
                                shader->setUniformValue("pointSize", getPointSize());
                            }
#ifndef ANDROID
                            else
                                f->glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
#endif
                            f->glUniform3fv(f->glGetUniformLocation(shader->programId(), "axisModifier"), 1, value_ptr(getAxisModifier()));

                            render(f);

                            drawn = true;
                        }
                        else emit modelLoadingDelayed();
                    }
                }
                shader->release();
            }
            else qDebug() << "Can't bind shader.";
        }
        else qDebug() << "No shader defined.";
        return drawn;
    }

    bool Model3D::drawBox()
    {
        if (isPrepared())
        {
            if (boxShader)
            {
                if (boxShader->bind())
                {
                    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
                    bool drawn = false;
                    if (isPrepared())
                    {
                        if (isBoxVisible())
                        {
                            QColor boxColor = Qt::gray;
                            if (isOnRAM())
                            {
                                if (isOnVRAM()) boxColor = Qt::green;
                                else boxColor = Qt::darkGreen;
                            }

                            loadBoxRAM();
                            loadBoxVRAM();
                            if (boxOnVRAM)
                            {
                                mat4 wMo = getwMo();
                                glm::vec3 color = glm::vec3(boxColor.redF(), boxColor.greenF(), boxColor.blueF());
                                f->glUniformMatrix4fv(f->glGetUniformLocation(boxShader->programId(), "wMo"), 1, GL_FALSE, value_ptr(wMo));
                                boxShader->setUniformValue("scale", getScale());
                                f->glUniform3fv(f->glGetUniformLocation(boxShader->programId(), "color"), 1, value_ptr(color));
                                if (isOpacityEnabled()) boxShader->setUniformValue("opacity", getOpacity());
                                else boxShader->setUniformValue("opacity", 1.0f);
                                f->glUniform3fv(f->glGetUniformLocation(boxShader->programId(), "axisModifier"), 1, value_ptr(getAxisModifier()));

                                boxBuffer.bind();
                                boxShader->enableAttributeArray("in_vertex");
                                boxShader->setAttributeBuffer("in_vertex", GL_FLOAT, 0, 3);
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
                                boxShader->disableAttributeArray("in_vertex");

                                drawn = true;
                            }
                        }
                    }
                    boxShader->release();
                    return drawn;
                }
                else
                {
                    qDebug() << "Can't bind box shader.";
                    return false;
                }
            }
            else
            {
                qDebug() << "No box shader defined.";
                return false;
            }
        }
        else return false;
    }

    void Model3D::setScale(double _scale)
    {
        scale = _scale;
        emit modelMoved();
    }

    void Model3D::setPointSizeEnabled(bool enabled)
    {
        if (pointSizeEnabled != enabled)
        {
            pointSizeEnabled = enabled;
            emit modelRenderChanged();
        }
    }

    void Model3D::setPointSize(float pointSize)
    {
        if (this->pointSize != pointSize)
        {
            this->pointSize = pointSize;
            if(pointSizeEnabled)
                emit modelRenderChanged();
        }
    }

    void Model3D::setAxisModifier(vec3 axisModifier)
    {
        this->axisModifier = axisModifier;
        emit modelChanged();
        emit modelRenderChanged();
    }

    void Model3D::setwMo(mat4 wMo)
    {
        setPose(wMo);
    }

    void Model3D::setGlobalColorEnabled(bool enabled)
    {
        globalColorEnabled = enabled;
        emit modelRenderChanged();
    }

    void Model3D::setGlobalColor(QColor color)
    {
        globalColor = color;
        emit modelRenderChanged();
    }

    void Model3D::setOpacityEnabled(bool enabled)
    {
        if (opacityEnabled != enabled)
        {
            opacityEnabled = enabled;
            emit modelRenderChanged();
        }
    }

    void Model3D::setOpacity(float value)
    {
        if (opacity != value)
        {
            opacity = value;
            emit modelRenderChanged();
        }
    }

    void Model3D::setBlendFunction(BlendFunction function)
    {
        if (blendFunction != function)
        {
            blendFunction = function;
            emit modelRenderChanged();
        }
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
}
