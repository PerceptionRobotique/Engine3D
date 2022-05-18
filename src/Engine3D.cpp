#include "Engine3D.h"

#ifdef __linux__
void initEngine3DResources() //needs to be launch outside of any namespace
{
    Q_INIT_RESOURCE(Engine3D);
}
#endif

namespace MIS
{

    Engine3D::Engine3D(RenderMode _renderMode, QObject* parent)
        : QObject(parent)
        , renderMode(_renderMode)
        , initialized(false)
        , surface(nullptr, this)
        , context(nullptr)
        , boxShader(nullptr)
        , cameras(1, new Camera)
        , mainCamera(cameras.first())
#ifdef HAVE_VR
        , vrCameras(2, nullptr)
#endif
        , frameCounter(false)
        , pointSize(1.0f)
        , lineWidth(3.0f)
        , opacityEnabled(false)
        , opacity(1.0f)
        , blendFunction(BLEND_1)
        , renderAsked(false)
        , maxSamples(-1)
        , viewDistance(150.0f)
        , viewDistanceEnabled(true)
        , waitLoading(false)
        , maxDepth(-1)
        , maxMovingDepth(-1)
        , isMoving(false)
        , maxVertexLimitEnabled(true)
        , maxVertexLimit(100000000)
        , maxVertexToVRAMEnabled(true)
        , maxVertexToVRAM(1000000)
        , updateNextAsked(false)
        , updateModelsNextAsked(false)
        , breakModelsUpdater(false)
    {
#ifdef __linux__
        initEngine3DResources();
#endif
        switch (renderMode)
        {
        case NONE:
            break;

        case DIRECT:
            surface.create();
            context = new QOpenGLContext(this);
            break;

        case THREADED:
            surface.create();
            context = new QOpenGLContext(this);
            moveToThread(&renderThread);
            renderThread.start();
            break;
        }

        connect(this, SIGNAL(askRenderMode(RenderMode)), this, SLOT(setRenderMode(RenderMode)));
        connect(this, SIGNAL(askInitialization()), this, SLOT(initialize()));
        connect(this, SIGNAL(askClose(unsigned int)), this, SLOT(closeModel(unsigned int)));
        connect(mainCamera, SIGNAL(cameraChanged()), this, SLOT(nextModelsUpdate()));
        connect(mainCamera, SIGNAL(cameraChanged()), this, SIGNAL(askUpdate()));
        connect(this, SIGNAL(askRender()), this, SLOT(render()));
        connect(this, SIGNAL(askPicture()), this, SLOT(takePicture()));
        connect(this, SIGNAL(askPFM()), this, SLOT(takePFM()));
        connect(this, SIGNAL(askDestroy()), this, SLOT(destroy()));
        connect(this, SIGNAL(askPointSizeEnabled(bool)), this, SLOT(setPointSizeEnabled(bool)));
        connect(this, SIGNAL(askPointSize(double)), this, SLOT(setPointSize(double)));
        connect(this, SIGNAL(askLineWidth(float)), this, SLOT(setLineWidth(float)));
        connect(this, SIGNAL(askOpacityEnabled(bool)), this, SLOT(setOpacityEnabled(bool)));
        connect(this, SIGNAL(askOpacity(float)), this, SLOT(setOpacity(float)));
        connect(this, SIGNAL(askBlendFunction(BlendFunction)), this, SLOT(setBlendFunction(BlendFunction)));

#ifdef HAVE_VR
        connect(&vrTimer, SIGNAL(timeout()), this, SLOT(update()));
        connect(this, SIGNAL(askStopVR()), this, SLOT(stopVR()));
#endif
    }

    Engine3D::~Engine3D()
    {
        if (renderMode == THREADED)
        {
            renderThread.exit();
            renderThread.wait();
        }
    }

    bool Engine3D::isInitialized() const
    {
        return initialized;
    }

    bool Engine3D::isOnCamera(const Model3D* model, const Camera* camera) const
    {
        bool onScreen = false;
        if (camera)
        {
            onScreen |= camera->isActive();
            if (onScreen)
            {
                onScreen &= camera->cullingTest(model);
                const Octree* octree = dynamic_cast<const Octree*>(model);
                if (octree)
                {
                    onScreen &= octree->getDepth() <= octree->getMaxVisibleDepth();
                    if (maxDepth != -1) onScreen &= octree->getDepth() <= maxDepth;
                    if (viewDistanceEnabled)
                    {
                        if (octree->getDepth() > ceil((float)octree->getMaxDepth() * (1 - camera->distanceWith(octree) / viewDistance)))
                            onScreen = false;
                    }
                }
            }
        }
        return onScreen;
    }

    bool Engine3D::isOnScreen(const Model3D* model)
    {
        if (!maxVertexLimitEnabled || currentVertexNumber + model->getVertexNumber() <= maxVertexLimit)
        {
            bool onScreen = false;
            for (Camera* camera : cameras) onScreen |= isOnCamera(model, camera);
#ifdef HAVE_VR
            if (vr.isActive()) for (Camera* vrCamera : vrCameras) onScreen |= isOnCamera(model, vrCamera);
#endif
            if (onScreen) currentVertexNumber += model->getVertexNumber();
            return onScreen;
        }
        else
            return false;
    }

    Engine3D::RenderMode Engine3D::getRenderMode() const
    {
        return renderMode;
    }

    QOpenGLContext* Engine3D::getContext()
    {
        return context;
    }

    QImage Engine3D::getFrame()
    {
        QImage output;
        frameMutex.lock();
        output = frame;
        frameMutex.unlock();
        return output;
    }

    Camera* Engine3D::getMainCamera()
    {
        return mainCamera;
    }

    QVector<Camera*>& Engine3D::getCameras()
    {
        return cameras;
    }

    int Engine3D::getMaxSamples()
    {
        return maxSamples;
    }

    Model3D* Engine3D::getModel(unsigned int index)
    {
        return models[index];
    }

    int Engine3D::getModelIndex(const Model3D* _model) const
    {
        int index = -1;
        for (unsigned int i = 0; i < models.count(); i++)
        {
            if (_model == models[i]) index = i;
        }
        return index;
    }

    QVector<Model3D*>& Engine3D::getModels()
    {
        return models;
    }

    float Engine3D::getOpacity() const
    {
        return opacity;
    }

    bool Engine3D::getWaitLoading() const
    {
        return waitLoading;
    }

    void Engine3D::lockDraw()
    {
        drawMutex.lock();
    }

    void Engine3D::unlockDraw()
    {
        drawMutex.unlock();
    }

    void Engine3D::lockModelsUpdater()
    {
        modelsUpdaterMutex.lock();
    }

    void Engine3D::unlockModelsUpdater()
    {
        modelsUpdaterMutex.unlock();
    }

    void Engine3D::setRenderMode(RenderMode _renderMode)
    {
        //if (QThread::currentThread() != thread())
        //{
        //    QEventLoop loop;
        //    connect(this, SIGNAL(renderModeChanged()), &loop, SLOT(quit()));
        //    emit askRenderMode(_renderMode);
        //    loop.exec();
        //}
        //else
        //{
        //    renderMode = _renderMode;
        //    switch (renderMode)
        //    {
        //    case DIRECT:
        //    {
        //        destroy();
        //        //surface.deleteLater();
        //        renderThread.exit();
        //        moveToThread(QApplication::instance()->thread());
        //        cameras.append(new Camera);
        //        initialize();
        //        //context->setShareContext(QOpenGLContext::currentContext());
        //        //QOpenGLContext::currentContext()->create();
        //        //context->deleteLater();
        //        //context = nullptr;
        //        //setRenderAsked(false);
        //        break;
        //    }

        //    case THREADED:
        //        moveToThread(&renderThread);
        //        renderThread.start();
        //        break;
        //    }

        //    emit renderModeChanged();
        //}
    }

    void Engine3D::initialize()
    {
        if (QThread::currentThread() != thread())
            emit askInitialization();
        else
        {
            if (context) context->create();
            makeCurrent();
            initializeOpenGLFunctions();

            shaders[Model3D::POINTS] = new QOpenGLShaderProgram;
            shaders[Model3D::POINTS]->create();
            shaders[Model3D::POINTS]->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/Point.vert");
            shaders[Model3D::POINTS]->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/Point.frag");
            if (!shaders[Model3D::POINTS]->link()) qDebug() << "Can't link shader.";

            boxShader = new QOpenGLShaderProgram;
            boxShader->create();
            boxShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/Box.vert");
            boxShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/Box.frag");
            if (!boxShader->link()) qDebug() << "Can't link box shader.";

            glEnable(GL_DEPTH_TEST);
            glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
            doneCurrent();

            setPointSizeEnabled(true);
            setPointSize(pointSize);
            setLineWidth(lineWidth);
            setOpacityEnabled(opacityEnabled);
            setOpacity(opacity);
            setBlendFunction(blendFunction);

            initialized = true;
            emit initializationFinished();
        }
    }

    void Engine3D::setMainCamera(Camera* camera)
    {
        mainCamera = camera;
    }

    void Engine3D::addCamera(Camera* camera)
    {
        cameras.append(camera);
    }

    void Engine3D::removeCamera(unsigned int index)
    {
        if (index < cameras.count())
        {
            delete cameras[index];
            cameras.remove(index);
        }
    }

    void Engine3D::removeCamera(Camera* camera)
    {
        removeCamera(cameras.indexOf(camera));
    }

    void Engine3D::openModel(QString fileName)
    {
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix() == "pts")                                     models.append(new ModelPTS(fileName));
        else if (fileInfo.suffix() == "bin" || fileInfo.suffix() == "bini") models.append(new ModelBIN(fileName));
        else if (fileInfo.suffix() == "oct" || fileInfo.suffix() == "octi") models.append(new Octree(nullptr, fileName));

        //connect(models.last(), SIGNAL(modelLoaded()), this, SIGNAL(askUpdate()));
        //connect(models.last(), SIGNAL(modelUnloaded()), this, SIGNAL(askUpdate()));
        //connect(models.last(), SIGNAL(modelChanged()), this, SIGNAL(askUpdate()));
        //connect(models.last(), SIGNAL(modelLoadingDelayed()), this, SIGNAL(askUpdate()));
        //connect(models.last(), SIGNAL(modelDestroyed()), this, SIGNAL(askUpdate()));

        connect(models.last(), SIGNAL(modelLoaded()), this, SIGNAL(askUpdate()));
        connect(models.last(), SIGNAL(modelUnloaded()), this, SIGNAL(askUpdate()));
        connect(models.last(), SIGNAL(modelChanged()), this, SIGNAL(askUpdate()));
        connect(models.last(), SIGNAL(modelLoadingDelayed()), this, SLOT(nextModelsUpdate()));
        connect(models.last(), SIGNAL(modelDestroyed()), this, SLOT(nextModelsUpdate()));

        nextModelsUpdate();
    }

    void Engine3D::closeModel(unsigned int index)
    {
        if (QThread::currentThread() != thread())
            emit askClose(index);
        else
        {
            modelsUpdaterMutex.lock();
            makeCurrent();
            delete models[index];
            models.removeAt(index);
            doneCurrent();
            modelsUpdaterMutex.unlock();
        }
    }

    void Engine3D::closeModel(Model3D* model)
    {
        int index = getModelIndex(model);
        if (index >= 0) closeModel(index);
    }

    void Engine3D::update()
    {
        if (initialized)
        {
            if (!getRenderAsked())
            {
                setRenderAsked(true);
#ifdef HAVE_VR
                if (vr.isActive()) emit updateVRInputs();
#endif
                emit askRender();
            }
            else updateNextAsked = true;
        }
    }

    QImage Engine3D::takePicture()
    {
        bool wasWaitLoading = getWaitLoading();
        setWaitLoading(true);
        if (QThread::currentThread() != thread())
        {
            QEventLoop loop;
            connect(this, SIGNAL(pictureTaken()), &loop, SLOT(quit()));
            emit askPicture();
            loop.exec();
            setWaitLoading(wasWaitLoading);
            return pictureAsked;
        }
        else
        {
            render();
            pictureAsked = getFrame();
            setWaitLoading(wasWaitLoading);
            emit pictureTaken();
            return pictureAsked;
        }
    }

#ifdef HAVE_VISP
    vpImage<float> Engine3D::takePFM()
    {
        bool wasWaitLoading = getWaitLoading();
        setWaitLoading(true);
        if (QThread::currentThread() != thread())
        {
            QEventLoop loop;
            connect(this, SIGNAL(pictureTaken()), &loop, SLOT(quit()));
            emit askPFM();
            loop.exec();
            setWaitLoading(wasWaitLoading);
            return pfmAsked;
        }
        else
        {
            unsigned int samples = mainCamera->getSamples();
            mainCamera->setSamples(0);
            render();
            setWaitLoading(wasWaitLoading);

            makeCurrent();
            mainCamera->bind();

            QVector<float> buffer;
            buffer.resize(mainCamera->getWidth() * mainCamera->getHeight());

            glReadPixels(0, 0, mainCamera->getWidth(), mainCamera->getHeight(), GL_DEPTH_COMPONENT, GL_FLOAT, buffer.data());

            float zNear = mainCamera->getNearPlane();
            float zFar = mainCamera->getFarPlane();

            for (unsigned int h = 0; h < (unsigned int)mainCamera->getHeight(); h++)
            {
                for (unsigned int w = 0; w < (unsigned int)mainCamera->getWidth(); w++)
                {
                    if (buffer[h * mainCamera->getWidth() + w] > 0 && buffer[h * mainCamera->getWidth() + w] < 1)
                    {
                        switch (mainCamera->getProjectionType())
                        {
                        case Camera::PERSPECTIVE:
                            buffer[h * mainCamera->getWidth() + w] = 2.0f * zFar * zNear / ((zFar + zNear) - (buffer[h * mainCamera->getWidth() + w] * (zFar - zNear)));
                            break;

                        case Camera::ORTHOGRAPHIC:
                            break;

                        case Camera::EQUIRECTANGULAR:
                        {
                            buffer[h * mainCamera->getWidth() + w] = buffer[h * mainCamera->getWidth() + w] * (zFar - zNear) + zNear;
                            break;
                        }

                        default:
                            break;
                        }
                    }
                    else
                        buffer[h * mainCamera->getWidth() + w] = -1;
                }
            }

            pfmAsked = vpImage<float>(buffer.data(), mainCamera->getHeight(), mainCamera->getWidth(), true);
            vpImageTools::flip(pfmAsked);

            mainCamera->release();
            doneCurrent();

            mainCamera->setSamples(samples);

            emit pictureTaken();
            return pfmAsked;
        }
    }
#endif

#ifdef HAVE_VR
    bool Engine3D::startVR()
    {
        if (!vr.isActive())
        {
            if (vr.initOpenVR() == VRheadset::noErr)
            {
                QThreadPool::globalInstance()->setMaxThreadCount(QThreadPool::globalInstance()->maxThreadCount() / 2);

                mainCamera->setActive(false);
                vr.getEyeTransformations();

                for (unsigned int i = 0; i < 2; i++)
                {
                    vrCameras[i] = new Camera;
                    vrCameras[i]->setWidth(vr.getWidth());
                    vrCameras[i]->setHeight(vr.getHeight());
                    vrCameras[i]->setCustomProjection(i ? vr.rmatMVP : vr.lmatMVP);
                    vrCameras[i]->setProjectionType(Camera::CUSTOM);
                    vrCameras[i]->setSamples(getMaxSamples());
                }

                vrTimer.start(1000.0f / vr.m_frequency);
                return true;
            }
            else return false;
        }
        else return true;
    }

    void Engine3D::stopVR()
    {
        if (vr.isActive())
        {
            if (QThread::currentThread() != thread())
            {
                QEventLoop loop;
                connect(this, SIGNAL(vrStopped()), &loop, SLOT(quit()));
                emit askStopVR();
                loop.exec();
            }
            else
            {
                vrTimer.stop();
                vr.shutdown();
                vr.shutdown();

                QThreadPool::globalInstance()->setMaxThreadCount(QThreadPool::globalInstance()->maxThreadCount());

                for (unsigned int i = 0; i < 2; i++)
                {
                    delete vrCameras[i];
                    vrCameras[i] = nullptr;
                }

                mainCamera->setActive(true);
                emit vrStopped();
            }
        }
    }

    VRheadset* Engine3D::getVRheadset()
    {
        return &vr;
    }

    Camera* Engine3D::getVRCamera(unsigned int index)
    {
        return vrCameras[index];
    }
#endif

    void Engine3D::setFrameCounterEnabled(bool enabled)
    {
        frameCounter = enabled;
    }

    void Engine3D::setPointSizeEnabled(bool enabled)
    {
        if (QThread::currentThread() != thread())
            emit askPointSizeEnabled(enabled);
        else
        {
            makeCurrent();
            if (enabled) glEnable(GL_PROGRAM_POINT_SIZE);
            else glDisable(GL_PROGRAM_POINT_SIZE);
            doneCurrent();
            emit askUpdate();
        }
    }

    void Engine3D::setPointSize(double _pointSize)
    {
        if (QThread::currentThread() != thread())
            emit askPointSize(_pointSize);
        else
        {
            makeCurrent();
            pointSize = _pointSize;
            shaders[Model3D::POINTS]->bind();
            shaders[Model3D::POINTS]->setUniformValue("pointSize", pointSize);
            shaders[Model3D::POINTS]->release();
            doneCurrent();
            emit askUpdate();
        }
    }

    void Engine3D::setLineWidth(float _lineWidth)
    {
        if (QThread::currentThread() != thread())
            emit askLineWidth(_lineWidth);
        else
        {
            makeCurrent();
            lineWidth = _lineWidth;
            glLineWidth(lineWidth);
            doneCurrent();
            emit askUpdate();
        }
    }

    void Engine3D::setOpacityEnabled(bool enabled)
    {
        if (QThread::currentThread() != thread())
            emit askOpacityEnabled(enabled);
        else
        {
            opacityEnabled = enabled;
            if (opacityEnabled)
            {
                makeCurrent();
                glDisable(GL_DEPTH_TEST);
                glEnable(GL_BLEND);
                doneCurrent();
                setOpacity(opacity);
                setBlendFunction(blendFunction);
            }
            else
            {
                makeCurrent();
                glEnable(GL_DEPTH_TEST);
                glDisable(GL_BLEND);
                doneCurrent();
                setOpacity(opacity);
                setBlendFunction(blendFunction);
            }
            emit askUpdate();
        }
    }

    void Engine3D::setOpacity(float _opacity)
    {
        if (QThread::currentThread() != thread())
            emit askOpacity(_opacity);
        else
        {
            opacity = _opacity;
            makeCurrent();
            if (opacityEnabled)
            {
                shaders[Model3D::POINTS]->bind();
                shaders[Model3D::POINTS]->setUniformValue("opacity", opacity);
                shaders[Model3D::POINTS]->release();
            }
            else
            {
                shaders[Model3D::POINTS]->bind();
                shaders[Model3D::POINTS]->setUniformValue("opacity", 1.0f);
                shaders[Model3D::POINTS]->release();
            }
            doneCurrent();
            emit askUpdate();
        }
    }

    void Engine3D::setBlendFunction(BlendFunction _blendFunction)
    {
        if (QThread::currentThread() != thread())
            emit askBlendFunction(_blendFunction);
        else
        {
            blendFunction = _blendFunction;
            makeCurrent();
            if (opacityEnabled) glBlendFunc(GL_SRC_ALPHA, blendFunction);
            else glBlendFunc(GL_ONE, GL_ZERO);
            doneCurrent();
            emit askUpdate();
        }
    }

    void Engine3D::setViewDistance(double _viewDistance)
    {
        viewDistance = _viewDistance;
        nextModelsUpdate();
    }

    void Engine3D::setViewDistanceEnabled(bool enabled)
    {
        viewDistanceEnabled = enabled;
        nextModelsUpdate();
    }

    void Engine3D::setWaitLoading(bool enabled)
    {
        waitLoading = enabled;
    }

    void Engine3D::setMaxVertexLimitEnabled(bool enabled)
    {
        maxVertexLimitEnabled = enabled;
        nextModelsUpdate();
    }

    void Engine3D::setMaxVertexLimit(int _maxVertexLimit)
    {
        maxVertexLimit = _maxVertexLimit * 1000000;
        nextModelsUpdate();
    }

    void Engine3D::setMaxVertexToVRAMEnabled(bool enabled)
    {
        maxVertexToVRAMEnabled = enabled;
    }

    void Engine3D::setMaxVertexToVRAM(double _maxVertexToVRAM)
    {
        maxVertexToVRAM = _maxVertexToVRAM * 1000000;
    }

    void Engine3D::setMaxMovingDepth(int _maxMovingDepth)
    {
        maxMovingDepth = _maxMovingDepth;
    }

    void Engine3D::setMoving(bool _isMoving)
    {
        isMoving = _isMoving;
        if (!isMoving)
        {
            breakModelsUpdater = true;
            nextModelsUpdate();
        }
        emit askUpdate();
    }

    void Engine3D::sortModelsByDepthAndDistance(QHash<unsigned int, QMap<float, QList<Model3D*>>>& modelsByDepthAndDistance, QList<Model3D*>& modelsToUnload)
    {
        QMap<unsigned int, QMap<float, QList<Model3D*>>> totlaModelsByDepthAndDistance;
        for (Model3D* model : models)
        {
            float minDist = cameras[0]->distanceWith(model);
            for (Camera* camera : cameras) minDist = (camera->distanceWith(model) < minDist ? camera->distanceWith(model) : minDist);
            totlaModelsByDepthAndDistance[0][minDist].append(model);

            Octree* octree = dynamic_cast<Octree*>(model);
            if (octree)
            {
                for (unsigned int i = 1; i <= octree->getMaxDepth(); i++)
                {
                    for (Octree* child : octree->getDepthChildren(i))
                    {
                        minDist = cameras[0]->distanceWith(child);
                        for (Camera* camera : cameras) minDist = (camera->distanceWith(child) < minDist ? camera->distanceWith(child) : minDist);
                        totlaModelsByDepthAndDistance[child->getDepth()][minDist].append(child);
                        if (breakModelsUpdater) break;
                    }
                    if (breakModelsUpdater) break;
                }
            }
            if (breakModelsUpdater) break;
        }

        for (unsigned int depth : totlaModelsByDepthAndDistance.keys())
        {
            for (float dist : totlaModelsByDepthAndDistance[depth].keys())
            {
                QList<Model3D*>& modelsToTest = totlaModelsByDepthAndDistance[depth][dist];
                for (Model3D* model : modelsToTest)
                {
                    if (isOnScreen(model)) modelsByDepthAndDistance[depth][dist].append(model);
                    else if (model->isLiveLoading()) modelsToUnload.append(model);
                    //if (depth == 0)
                    //{
                    //    if (!onScreen)
                    //    {
                    //        qDebug() << "currentVertexNumber = " << currentVertexNumber;
                    //        qDebug() << "culling test = " << mainCamera->cullingTest(model);
                    //        Octree* octree = dynamic_cast<Octree*>(model);
                    //        if (octree)
                    //        {
                    //            qDebug() << "depthTest = " << octree->getDepth() << " <= " << octree->getMaxVisibleDepth();
                    //            qDebug() << "maxDepth = " << octree->getDepth() << " <= " << maxDepth;
                    //            qDebug() << "viewDistance = " << octree->getDepth() << " > " << ceil((float)octree->getMaxDepth() * (1 - mainCamera->distanceWith(octree) / viewDistance));
                    //            qDebug() << Qt::endl;
                    //        }
                    //    }
                    //}
                    if (breakModelsUpdater) break;
                }
                if (breakModelsUpdater) break;
            }
            if (breakModelsUpdater) break;
        }
    }

    void Engine3D::updateModels()
    {
        if (modelsUpdaterMutex.tryLock())
        {
            currentVertexNumber = 0;
            QList<Model3D*> modelsToUnload;
            QHash<unsigned int, QMap<float, QList<Model3D*>>> modelsByDepthAndDistance;
            sortModelsByDepthAndDistance(modelsByDepthAndDistance, modelsToUnload);

            QFuture<void> modelsUnloader = QtConcurrent::map(modelsToUnload, std::bind(&Model3D::unloadRAM, std::placeholders::_1, waitLoading));
            for (unsigned int depth : modelsByDepthAndDistance.keys())
            {
                for (float dist : modelsByDepthAndDistance[depth].keys())
                {
                    QList<Model3D*>& modelsToLoad = modelsByDepthAndDistance[depth][dist];
                    QtConcurrent::blockingMap(modelsToLoad, std::bind(&Model3D::loadRAM, std::placeholders::_1, waitLoading));
                    if (breakModelsUpdater) break;
                }
                if (breakModelsUpdater) break;
            }
            modelsUnloader.waitForFinished();
            modelsUpdaterMutex.unlock();
        }
        if (updateModelsNextAsked)
        {
            updateModelsNextAsked = false;
            if(!waitLoading) modelsUpdater = QtConcurrent::run(&Engine3D::updateModels, this);
        }
        breakModelsUpdater = false;
    }

    void Engine3D::makeCurrent()
    {
        if (context)
        {
            contextMutex.lock();
            context->makeCurrent(&surface);
        }
    }

    void Engine3D::doneCurrent()
    {
        if (context)
        {
            context->doneCurrent();
            contextMutex.unlock();
        }
    }

    void Engine3D::setRenderAsked(bool value)
    {
        renderAskedMutex.lock();
        renderAsked = value;
        renderAskedMutex.unlock();
    }

    bool Engine3D::getRenderAsked()
    {
        renderAskedMutex.lock();
        bool value = renderAsked;
        renderAskedMutex.unlock();
        return value;
    }

    void Engine3D::render()
    {
        if (initialized)
        {
            if (drawMutex.tryLock())
            {
                makeCurrent();
                if (waitLoading)
                {
                    if(modelsUpdater.isRunning()) modelsUpdater.waitForFinished();
                    updateModels();
                }

                static unsigned long long frameNumber = 0;
                unsigned long long vertexToVRAM = 0;

                for (Camera* camera : cameras)
                {
                    if (camera->bind())
                    {
                        if (shaders[Model3D::POINTS]->bind())
                        {
                            glViewport(0, 0, camera->getWidth(), camera->getHeight());
                            glClearColor(
                                camera->getBackgroundColor().redF(),
                                camera->getBackgroundColor().greenF(),
                                camera->getBackgroundColor().blueF(),
                                camera->getBackgroundColor().alphaF());
                            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                            shaders[Model3D::POINTS]->setUniformValue("equirectangular", camera->getProjectionType() == Camera::EQUIRECTANGULAR);
                            glUniformMatrix4fv(glGetUniformLocation(shaders[Model3D::POINTS]->programId(), "view"), 1, GL_FALSE, camera->getcMwPtr());
                            glUniformMatrix4fv(glGetUniformLocation(shaders[Model3D::POINTS]->programId(), "projection"), 1, GL_FALSE, camera->getProjectionPtr());

                            for (Model3D* model : models)
                            {
                                if (model->isOnRAM())
                                {
                                    if (model->isOnVRAM()) model->draw(shaders[Model3D::POINTS]);
                                    else if (!model->isOnVRAM() && vertexToVRAM <= maxVertexToVRAM || !maxVertexToVRAMEnabled || waitLoading)
                                    {
                                        vertexToVRAM += model->getVertexNumber();
                                        model->loadVRAM(waitLoading);
                                        model->draw(shaders[Model3D::POINTS]);
                                    }
                                    else emit askUpdate();
                                }
                                else model->unloadVRAM(waitLoading);
                                Octree* octree = dynamic_cast<Octree*>(model);
                                if (octree)
                                {
                                    for (unsigned int i = 1; i <= qMin(octree->getMaxDepth(), maxMovingDepth != -1 ? isMoving ? maxMovingDepth : octree->getMaxDepth() : octree->getMaxDepth()); i++)
                                    {
                                        for (Octree* child : octree->getDepthChildren(i))
                                        {
                                            if (child->isOnRAM())
                                            {
                                                if (child->isOnVRAM()) child->draw(shaders[Model3D::POINTS]);
                                                else if (!child->isOnVRAM() && vertexToVRAM <= maxVertexToVRAM || !maxVertexToVRAMEnabled || waitLoading)
                                                {
                                                    vertexToVRAM += child->getVertexNumber();
                                                    child->loadVRAM(waitLoading);
                                                    child->draw(shaders[Model3D::POINTS]);
                                                }
                                                else emit askUpdate();
                                            }
                                            else child->unloadVRAM(waitLoading);
                                        }
                                    }
                                }
                            }
                            shaders[Model3D::POINTS]->release();
                        }
                        else QMessageBox::warning(nullptr, "Error", "Can't bind shader.");

                        if (boxShader->bind())
                        {
                            glUniformMatrix4fv(glGetUniformLocation(boxShader->programId(), "view"), 1, GL_FALSE, camera->getcMwPtr());
                            glUniformMatrix4fv(glGetUniformLocation(boxShader->programId(), "projection"), 1, GL_FALSE, camera->getProjectionPtr());
                            for (Model3D* model : models)
                            {
                                if (camera->cullingTest(model)) model->drawBox(boxShader);
                                Octree* octree = dynamic_cast<Octree*>(model);
                                if (octree)
                                {
                                    for (unsigned int i = 1; i <= octree->getMaxDepth(); i++)
                                    {
                                        for (Octree* child : octree->getDepthChildren(i))
                                        {
                                            child->drawBox(boxShader);
                                        }
                                    }
                                }
                            }
                            boxShader->release();
                        }
                        else qDebug() << "Can't bind box shader.";
                        camera->release();
                    }
                }

                setFrame();

#ifdef HAVE_VR
                if (vr.isActive())
                {
                    vr.getEyeTransformations();
                    mainCamera->setRotation(vr.m_mat4eyePosLeft);
                    vrCameras[0]->setPosition(vec4(mainCamera->getPosition(), 1.0f) + vr.m_mat4eyePosLeft[3]);
                    vrCameras[1]->setPosition(vec4(mainCamera->getPosition(), 1.0f) + vr.m_mat4eyePosRight[3]);
                    vrCameras[0]->setRotation(vr.m_mat4eyePosLeft);
                    vrCameras[1]->setRotation(vr.m_mat4eyePosRight);

                    for (unsigned int i = 0 ; i < 2 ; i++)
                    {
                        if (vrCameras[i]->bind())
                        {
                            if (shaders[Model3D::POINTS]->bind())
                            {
                                glViewport(0, 0, vrCameras[i]->getWidth(), vrCameras[i]->getHeight());
                                glClearColor(
                                    vrCameras[i]->getBackgroundColor().redF(),
                                    vrCameras[i]->getBackgroundColor().greenF(),
                                    vrCameras[i]->getBackgroundColor().blueF(),
                                    vrCameras[i]->getBackgroundColor().alphaF());
                                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                                shaders[Model3D::POINTS]->setUniformValue("equirectangular", vrCameras[i]->getProjectionType() == Camera::EQUIRECTANGULAR);
                                glUniformMatrix4fv(glGetUniformLocation(shaders[Model3D::POINTS]->programId(), "view"), 1, GL_FALSE, vrCameras[i]->getcMwPtr());
                                glUniformMatrix4fv(glGetUniformLocation(shaders[Model3D::POINTS]->programId(), "projection"), 1, GL_FALSE, vrCameras[i]->getProjectionPtr());

                                for (Model3D* model : models)
                                {
                                    if (model->isOnRAM())
                                    {
                                        if (model->isOnVRAM()) model->draw(shaders[Model3D::POINTS]);
                                        else if (!model->isOnVRAM() && vertexToVRAM <= maxVertexToVRAM || !maxVertexToVRAMEnabled || waitLoading)
                                        {
                                            vertexToVRAM += model->getVertexNumber();
                                            model->loadVRAM(waitLoading);
                                            model->draw(shaders[Model3D::POINTS]);
                                        }
                                        else emit askUpdate();
                                    }
                                    else model->unloadVRAM(waitLoading);
                                    Octree* octree = dynamic_cast<Octree*>(model);
                                    if (octree)
                                    {
                                        for (unsigned int i = 1; i <= qMin(octree->getMaxDepth(), maxMovingDepth != -1 ? isMoving ? maxMovingDepth : octree->getMaxDepth() : octree->getMaxDepth()); i++)
                                        {
                                            for (Octree* child : octree->getDepthChildren(i))
                                            {
                                                if (child->isOnRAM())
                                                {
                                                    if (child->isOnVRAM()) child->draw(shaders[Model3D::POINTS]);
                                                    else if (!child->isOnVRAM() && vertexToVRAM <= maxVertexToVRAM || !maxVertexToVRAMEnabled || waitLoading)
                                                    {
                                                        vertexToVRAM += child->getVertexNumber();
                                                        child->loadVRAM(waitLoading);
                                                        child->draw(shaders[Model3D::POINTS]);
                                                    }
                                                    else emit askUpdate();
                                                }
                                                else child->unloadVRAM(waitLoading);
                                            }
                                        }
                                    }
                                }
                                shaders[Model3D::POINTS]->release();
                            }
                            else QMessageBox::warning(nullptr, "Error", "Can't bind shader.");

                            if (boxShader->bind())
                            {
                                glUniformMatrix4fv(glGetUniformLocation(boxShader->programId(), "view"), 1, GL_FALSE, vrCameras[i]->getcMwPtr());
                                glUniformMatrix4fv(glGetUniformLocation(boxShader->programId(), "projection"), 1, GL_FALSE, vrCameras[i]->getProjectionPtr());
                                for (Model3D* model : models)
                                {
                                    if (vrCameras[i]->cullingTest(model)) model->drawBox(boxShader);
                                    Octree* octree = dynamic_cast<Octree*>(model);
                                    if (octree)
                                    {
                                        for (unsigned int i = 1; i <= octree->getMaxDepth(); i++)
                                        {
                                            for (Octree* child : octree->getDepthChildren(i))
                                            {
                                                child->drawBox(boxShader);
                                            }
                                        }
                                    }
                                }
                                boxShader->release();
                            }
                            else qDebug() << "Can't bind box shader.";
                            vrCameras[i]->release();

                            vr::Texture_t eyeTexture = { (void*)(uintptr_t)vrCameras[i]->texture(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
                            vr::VRCompositor()->Submit(i ? vr::Eye_Right : vr::Eye_Left, & eyeTexture);
                        }
                    }
                }
#endif

                if (frameCounter) qDebug() << ++frameNumber;

                GLenum err;
                while ((err = glGetError()) != GL_NO_ERROR) qDebug() << err;

                doneCurrent();
                setRenderAsked(false);
                drawMutex.unlock();
                if (updateNextAsked)
                {
                    updateNextAsked = false;
                    emit askUpdate();
                }
            }
            else updateNextAsked = true;
        }
    }

    void Engine3D::nextModelsUpdate()
    {
        if (!modelsUpdater.isRunning())
            modelsUpdater = QtConcurrent::run(&Engine3D::updateModels, this);
        else
            updateModelsNextAsked = true;
    }

    void Engine3D::setFrame()
    {
        if (mainCamera->isActive())
        {
            frameMutex.lock();
            frame = mainCamera->toImage();
            frameMutex.unlock();
            emit frameReady(frame);
        }
    }

    void Engine3D::destroy()
    {
        initialized = false;
        if (QThread::currentThread() != thread())
        {
            QEventLoop loop;
            connect(this, SIGNAL(destructionFinished()), &loop, SLOT(quit()));
            emit askDestroy();
            loop.exec();
        }
        else
        {
            modelsUpdaterMutex.lock();
            makeCurrent();
#ifdef HAVE_VR
            if (vr.isActive()) stopVR();
#endif
            for (Camera* camera : cameras) delete camera;
            for (QOpenGLShaderProgram* shader : shaders) delete shader;
            delete boxShader;
            for (Model3D* model : models) delete model;
            cameras.clear();
            shaders.clear();
            models.clear();
            doneCurrent();
            if (context)
            {
                delete context;
                context = nullptr;
            }
            modelsUpdaterMutex.unlock();
            emit destructionFinished();
        }
    }

}