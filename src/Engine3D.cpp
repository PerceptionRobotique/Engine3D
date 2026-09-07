#include "Engine3D.h"

#ifdef __linux__
/**
 * @brief      Initializes the engine 3D resources (Linux only).
 */
void initEngine3DResources() //needs to be launch outside of any namespace
{
    Q_INIT_RESOURCE(Engine3D);
}
#endif

namespace MIS
{
    /// <summary>
    /// Create Engine3D instance.
    /// </summary>
    /// <param name="_renderMode">Select how the engine manages OpenGL context
    /// - NONE : no OpenGL context (useful when a widget already has an OpenGL context)
    /// - DIRECT : renders models directly (useful for scripted programs)
    /// - THREADED : renders models directly and send frameReady(QImage) signals when frame is ready (useful for GUI so the interface will never be stucked while rendering)</param>
    /// <param name="parent">parent object</param>
    Engine3D::Engine3D(RenderMode _renderMode, QObject* parent)
        : QObject(parent)
        , renderMode(_renderMode)
        , initialized(false)
        , surface(nullptr, this)
        , context(nullptr)
        , boxShader(nullptr)
        , cameras(1, new Camera)
        , mainCamera(cameras.first())
#ifdef HAVE_OpenVR
        , vrCameras(2, nullptr)
        , eyesFBO(nullptr)
        , vrHandsEnabled(true)
        , hands(2, nullptr)
        , vrFloorEnabled(true)
        , floor(nullptr)
        , headLocked(false)
#endif
        , frameCounter(false)
        , lineWidth(3.0f)
        , lightOnMainCamera(true)
        , lightPosition(0, 5, 0)
        , globalIllumination(0.1f)
        , faceColor(0.5, 0.5, 0.5)
        , renderAsked(false)
        , maxSamples(-1)
        , viewDistance(150.0f)
        , viewDistanceEnabled(true)
        , waitLoading(false)
        , maxDepth(-1)
        , maxMovingDepth(0)
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
        connect(mainCamera, SIGNAL(cameraMoved()), this, SLOT(nextModelsUpdate()));
        connect(mainCamera, SIGNAL(cameraMoved()), this, SIGNAL(askUpdate()));
        connect(this, SIGNAL(askRender()), this, SLOT(render()));
        connect(this, SIGNAL(askPicture()), this, SLOT(takePicture()));
        connect(this, SIGNAL(askDepth(unsigned int, unsigned int)), this, SLOT(getDepth(unsigned int, unsigned int)));
        connect(this, SIGNAL(askDepthMap()), this, SLOT(takeDepthMap()));
#ifdef HAVE_ViSP
        connect(this, SIGNAL(askPFM()), this, SLOT(takePFM()));
#endif
        connect(this, SIGNAL(askDestroy()), this, SLOT(destroy()));
        connect(this, SIGNAL(askLineWidth(float)), this, SLOT(setLineWidth(float)));
        connect(this, SIGNAL(askLightOnMainCamera(bool)), this, SLOT(setLightOnMainCamera(bool)));
        connect(this, SIGNAL(askLightPosition(vec3)), this, SLOT(setLightPosition(vec3)));
        connect(this, SIGNAL(askGlobalIllumination(float)), this, SLOT(setGlobalIllumination(float)));
        connect(this, SIGNAL(askFaceColor(vec3)), this, SLOT(setFaceColor(vec3)));
            
#ifdef HAVE_OpenVR
        if(renderMode == NONE) connect(&vrTimer, SIGNAL(timeout()), this, SIGNAL(askUpdate()));
        else connect(&vrTimer, SIGNAL(timeout()), this, SLOT(update()));
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

    /**
     * @brief      Determines if the engine is initialized.
     *
     * @return     True if initialized, False otherwise.
     */
    bool Engine3D::isInitialized() const
    {
        return initialized;
    }

    /**
     * @brief      Determines whether the specified model is visible.
     *
     * @param[in]  model  The model
     *
     * @return     True if the specified model is visible, False otherwise.
     */
    bool Engine3D::isModelVisible(const Model3D* model) const
    {
        bool visible = false;
        for (Camera* camera : cameras)
            visible |= camera->isModelVisible(model);
#ifdef HAVE_OpenVR
        if (vr.isActive()) for (Camera* vrCamera : vrCameras) visible |= vrCamera->isModelVisible(model);
#endif
        return visible;
    }

    /**
     * @brief      Determines specified model is visible by specified camera.
     *
     * @param[in]  model   The model
     * @param[in]  camera  The camera
     *
     * @return     True if on camera, False otherwise.
     */
    bool Engine3D::isOnCamera(const Model3D* model, const Camera* camera) const
    {
        bool onScreen = false;
        if (camera)
        {
            onScreen |= camera->isActive();
            onScreen &= model->isVisible();
            if (onScreen)
            {
                onScreen &= camera->isModelVisible(model);
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

    /**
     * @brief      Determines whether the specified model is on screen.
     *
     * @param[in]  model  The model
     *
     * @return     True if the specified model is on screen, False otherwise.
     */
    bool Engine3D::isOnScreen(const Model3D* model)
    {
        if (!maxVertexLimitEnabled || currentVertexNumber + model->getVertexNumber() <= maxVertexLimit)
        {
            bool onScreen = false;
#ifdef HAVE_OpenVR
            if (vr.isActive() && !headLocked)
            {
                for (Camera* vrCamera : vrCameras)
                    onScreen |= isOnCamera(model, vrCamera);
                if (onScreen) currentVertexNumber += model->getVertexNumber();
            }
            else
            {
#endif
                for (Camera* camera : cameras)
                    onScreen |= isOnCamera(model, camera);
                if (onScreen) currentVertexNumber += model->getVertexNumber();
#ifdef HAVE_OpenVR
            }
#endif
            return onScreen;
        }
        else
            return false;
    }

    /**
     * @brief      Gets the render mode.
     *
     * @return     The render mode.
     */
    Engine3D::RenderMode Engine3D::getRenderMode() const
    {
        return renderMode;
    }

    /**
     * @brief      Gets the current OpenGL context.
     *
     * @return     The current OpenGL context.
     */
    QOpenGLContext* Engine3D::getContext()
    {
        return context;
    }

    QHash<Model3D::Primitives, QOpenGLShaderProgram*> Engine3D::getShaders()
    {
        return shaders;
    }

    QOpenGLShaderProgram* Engine3D::getBoxShader()
    {
        return boxShader;
    }

    /**
     * @brief      Gets the last frame generated.
     *
     * @return     The last frame generated.
     */
    QImage Engine3D::getFrame()
    {
        QImage output;
        frameMutex.lock();
        output = frame;
        frameMutex.unlock();
        return output;
    }

    /**
     * @brief      Gets the main camera.
     *
     * @return     The main camera.
     */
    Camera* Engine3D::getMainCamera()
    {
        return mainCamera;
    }

    /**
     * @brief      Gets the cameras.
     *
     * @return     The cameras.
     */
    QVector<Camera*>& Engine3D::getCameras()
    {
        return cameras;
    }

    /**
     * @brief      Gets the maximum samples for this device.
     *
     * @return     The maximum samples available.
     */
    int Engine3D::getMaxSamples()
    {
        return maxSamples;
    }

    /**
     * @brief      Gets the model by index.
     *
     * @param[in]  index  The index
     *
     * @return     The model.
     */
    Model3D* Engine3D::getModel(unsigned int index)
    {
        return models[index];
    }

    Model3D* Engine3D::getModel(const QString& name)
    {
        Model3D* modelFound = nullptr;
        for (Model3D* model : models)
            if (model->getName() == name)
                modelFound = model;
        return modelFound;
    }

    /**
     * @brief      Gets the index from the model.
     *
     * @param[in]  _model  The model
     *
     * @return     The model index.
     */
    int Engine3D::getModelIndex(const Model3D* _model) const
    {
        int index = -1;
        for (unsigned int i = 0; i < models.count(); i++)
        {
            if (_model == models[i]) index = i;
        }
        return index;
    }

    /**
     * @brief      Gets the models.
     *
     * @return     The models.
     */
    QVector<Model3D*>& Engine3D::getModels()
    {
        return models;
    }

    /**
     * @brief      Gets the wait loading parameter.
     *
     * @return     The wait loading parameter.
     */
    bool Engine3D::getWaitLoading() const
    {
        return waitLoading;
    }

    /**
     * @brief      Determines if view distance enabled.
     *
     * @return     True if view distance enabled, False otherwise.
     */
    bool Engine3D::isViewDistanceEnabled() const
    {
        return viewDistanceEnabled;
    }

    /**
     * @brief      Determines if view distance is enabled.
     *
     * @return     True if view distance is enabled, False otherwise.
     */
    float Engine3D::getViewDistance() const
    {
        return viewDistance;
    }

    /**
     * @brief      Determines if maximum vertex limit enabled.
     *
     * @return     True if maximum vertex limit enabled, False otherwise.
     */
    bool Engine3D::isMaxVertexLimitEnabled() const
    {
        return maxVertexLimitEnabled;
    }

    float Engine3D::getMaxVertexLimit() const
    {
        return maxVertexLimit / 1000000;
    }

    /**
     * @brief      Gets the default face color parameter.
     *
     * @return     The default face color parameter.
     */
    vec3 Engine3D::getFaceColor() const
    {
        return faceColor;
    }

    /**
     * @brief      Locks the draw function.
     */
    void Engine3D::lockDraw()
    {
        drawMutex.lock();
    }

    /**
     * @brief      Unlocks the draw function.
     */
    void Engine3D::unlockDraw()
    {
        drawMutex.unlock();
    }

    /**
     * @brief      Locks the models updater thread.
     */
    void Engine3D::lockModelsUpdater()
    {
        modelsUpdaterMutex.lock();
    }

    /**
     * @brief      Unlocks the models updater thread.
     */
    void Engine3D::unlockModelsUpdater()
    {
        modelsUpdaterMutex.unlock();
    }

    /**
     * @brief      Sets the render mode without requiring a restart (don't work yet, may be deleted in a close future).
     *
     * @param[in]  _renderMode  The render mode
     */
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

    /**
     * @brief      Initializes the Engine.
     */
    void Engine3D::initialize()
    {
//        QFuture<QtAndroidPrivate::PermissionResult> f = QtAndroidPrivate::requestPermission(QtAndroidPrivate::Storage);
//        QFuture<QtAndroidPrivate::PermissionResult> f = QtAndroidPrivate::requestPermission({"android.permission.WRITE_EXTERNAL_STORAGE"});
//        f.waitForFinished();
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

            shaders[Model3D::TRIANGLES] = new QOpenGLShaderProgram;
            shaders[Model3D::TRIANGLES]->create();
            shaders[Model3D::TRIANGLES]->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/AdvancedOBJ.vert");
            shaders[Model3D::TRIANGLES]->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/AdvancedOBJ.frag");
            if (!shaders[Model3D::TRIANGLES]->link()) qDebug() << "Can't link shader.";

            boxShader = new QOpenGLShaderProgram;
            boxShader->create();
            boxShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/Box.vert");
            boxShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/Box.frag");
            if (!boxShader->link()) qDebug() << "Can't link box shader.";

#ifndef ANDROID
            glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
#endif
            doneCurrent();

            setLineWidth(lineWidth);
            setLightOnMainCamera(lightOnMainCamera);
            setLightPosition(lightPosition);
            setGlobalIllumination(globalIllumination);
            setFaceColor(faceColor);

#ifdef HAVE_OpenVR
            hands[0] = new ModelOBJ(":/Models/oculus_controller_L.obj", shaders[Model3D::TRIANGLES], boxShader);
            hands[1] = new ModelOBJ(":/Models/oculus_controller_R.obj", shaders[Model3D::TRIANGLES], boxShader);
            floor = new ModelOBJ(":/Models/floor.obj", shaders[Model3D::TRIANGLES], boxShader);
            floor->setOpacityEnabled(true);
            floor->setOpacity(0.2f);
#endif

            initialized = true;
            emit initializationFinished();
        }
    }

    /**
     * @brief      Sets the main camera.
     *
     * @param      camera  The camera
     */
    void Engine3D::setMainCamera(Camera* camera)
    {
        if (!camera) return;
        mainCamera = camera;
        // The render loop and visibility tests only iterate over "cameras", so a
        // main camera set from the outside must be part of that list to actually
        // be rendered (and to have a valid frame in takePicture()/getFrame()).
        if (!cameras.contains(camera))
            cameras.append(camera);
    }

    /**
     * @brief      Adds a camera.
     *
     * @param      camera  The camera
     */
    void Engine3D::addCamera(Camera* camera)
    {
        cameras.append(camera);
    }

    /**
     * @brief      Removes a camera.
     *
     * @param[in]  index  The index of the camera
     */
    void Engine3D::removeCamera(unsigned int index)
    {
        if (index < cameras.count())
        {
            Camera* removed = cameras[index];
            cameras.remove(index);
            if (mainCamera == removed)
                mainCamera = cameras.isEmpty() ? nullptr : cameras.first();
            delete removed;
        }
    }

    /**
     * @brief      Removes a camera.
     *
     * @param      camera  The camera
     */
    void Engine3D::removeCamera(Camera* camera)
    {
        removeCamera(cameras.indexOf(camera));
    }

    void Engine3D::addModel(Model3D* model)
    {
        models.append(model);
        connect(models.last(), SIGNAL(modelLoaded()), this, SIGNAL(askUpdate()));
        connect(models.last(), SIGNAL(modelUnloaded()), this, SIGNAL(askUpdate()));
        connect(models.last(), SIGNAL(modelChanged()), this, SLOT(nextModelsUpdate()));
        connect(models.last(), SIGNAL(modelRenderChanged()), this, SIGNAL(askUpdate()));
        connect(models.last(), SIGNAL(modelMoved()), this, SIGNAL(askUpdate()));
        connect(models.last(), SIGNAL(modelMoved()), this, SLOT(nextModelsUpdate()));
        connect(models.last(), SIGNAL(modelLoadingDelayed()), this, SLOT(nextModelsUpdate()));
        connect(models.last(), SIGNAL(modelDestroyed()), this, SLOT(nextModelsUpdate()));

        nextModelsUpdate();
    }

    /**
     * @brief      Opens a model.
     *
     * @param[in]  fileName  The file name of the model
     */
    void Engine3D::openModel(QString fileName)
    {
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix() == "pts")                                     models.append(new ModelPTS(fileName, shaders[Model3D::POINTS], boxShader));
        else if (fileInfo.suffix() == "bin" || fileInfo.suffix() == "bini") models.append(new ModelBIN(fileName, shaders[Model3D::POINTS], boxShader));
        else if (fileInfo.suffix() == "oct" || fileInfo.suffix() == "octi") models.append(new Octree(nullptr, fileName, shaders[Model3D::POINTS], boxShader));
        else if (fileInfo.suffix() == "obj")                                models.append(new ModelOBJ(fileName, shaders[Model3D::TRIANGLES], boxShader));

        connect(models.last(), SIGNAL(modelLoaded()), this, SIGNAL(askUpdate()));
        connect(models.last(), SIGNAL(modelUnloaded()), this, SIGNAL(askUpdate()));
        connect(models.last(), SIGNAL(modelChanged()), this, SLOT(nextModelsUpdate()));
        connect(models.last(), SIGNAL(modelRenderChanged()), this, SIGNAL(askUpdate()));
        connect(models.last(), SIGNAL(modelMoved()), this, SIGNAL(askUpdate()));
        connect(models.last(), SIGNAL(modelMoved()), this, SLOT(nextModelsUpdate()));
        connect(models.last(), SIGNAL(modelLoadingDelayed()), this, SLOT(nextModelsUpdate()));
        connect(models.last(), SIGNAL(modelDestroyed()), this, SLOT(nextModelsUpdate()));

        nextModelsUpdate();
    }

    /**
     * @brief      Closes a model.
     *
     * @param[in]  index  The index of the model
     */
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
            nextModelsUpdate();
        }
    }

    /**
     * @brief      Closes a model.
     *
     * @param      model  The model to close
     */
    void Engine3D::closeModel(Model3D* model)
    {
        int index = getModelIndex(model);
        if (index >= 0) closeModel(index);
    }

    /**
     * @brief      Asks a render update.
     */
    void Engine3D::update()
    {
        if (isInitialized())
        {
            if (!getRenderAsked())
            {
                setRenderAsked(true);
#ifdef HAVE_OpenVR
                if (vr.isActive())
                {
                    emit updateVRInputs();
                }
#endif
                emit askRender();
            }
            else updateNextAsked = true;
        }
    }

    /**
     * @brief      Asks a full render in an image.
     *
     * @return     The image.
     */
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

    float Engine3D::getDepth(unsigned int h, unsigned int w)
    {
        bool wasWaitLoading = getWaitLoading();
        setWaitLoading(true);
        if (QThread::currentThread() != thread())
        {
            QEventLoop loop;
            connect(this, SIGNAL(pictureTaken()), &loop, SLOT(quit()));
            emit askDepth(h, w);
            loop.exec();
            setWaitLoading(wasWaitLoading);
            return depthAsked;
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
            buffer.resize(1);

            glReadPixels(w, mainCamera->getHeight() - h - 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, buffer.data());
            depthAsked = buffer[0];

            mainCamera->release();
            doneCurrent();

            mainCamera->setSamples(samples);

            emit pictureTaken();
            return depthAsked;
        }
    }

    float Engine3D::getDepthMeter(unsigned int h, unsigned int w)
    {
        float depth = getDepth(h, w);
        float depthMeter;

        float zNear = mainCamera->getNearPlane();
        float zFar = mainCamera->getFarPlane();
        if (depth < 1)
        {
            switch (mainCamera->getProjectionType())
            {
            case Camera::PERSPECTIVE:
                depthMeter = 2.0f * zFar * zNear / ((zFar + zNear) - (depth * (zFar - zNear)));
                break;

            case Camera::ORTHOGRAPHIC:
                break;

            case Camera::EQUIRECTANGULAR:
            {
                depthMeter = depth * (zFar - zNear) + zNear;
                break;
            }

            default:
                break;
            }
        }
        else
            depthMeter = -1;

        return depthMeter;
    }

    QVector<QVector<float>> Engine3D::takeDepthMap()
    {
        bool wasWaitLoading = getWaitLoading();
        setWaitLoading(true);
        if (QThread::currentThread() != thread())
        {
            QEventLoop loop;
            connect(this, SIGNAL(pictureTaken()), &loop, SLOT(quit()));
            emit askDepthMap();
            loop.exec();
            setWaitLoading(wasWaitLoading);
            return depthMapAsked;
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

            depthMapAsked.clear();
            depthMapAsked.resize(mainCamera->getHeight());
            for (unsigned int h = 0; h < mainCamera->getHeight(); h++)
            {
                for (unsigned int w = 0; w < mainCamera->getWidth(); w++)
                {
                    depthMapAsked[mainCamera->getHeight() - h - 1].resize(mainCamera->getWidth());
                    depthMapAsked[mainCamera->getHeight() - h - 1][w] = buffer[w + h * mainCamera->getWidth()];
                }
            }

            mainCamera->release();
            doneCurrent();

            mainCamera->setSamples(samples);

            emit pictureTaken();
            return depthMapAsked;
        }
    }

    vec4 Engine3D::getNearestPoint(unsigned int h, unsigned int w, unsigned int maxDist)
    {
        QVector<QVector<float>> depthMap = takeDepthMeterMap();
        vec4 point(0, 0, 0, 1);
        vec4 pixel(h, w, 0, 1);
        float z = depthMap[pixel.x][pixel.y];
        unsigned int dist = 1;
        unsigned int face = 0; //face du carré de contour
        int ow = 0;
        int oh = -(z == -1);
        while (z == -1 && dist <= maxDist && dist <= qMax(mainCamera->getWidth(), mainCamera->getHeight()))
        {
            //qDebug() << dist << " : " << pixel.x + oh << " x " << pixel.y + ow;
            if(pixel.x + oh >= 0 && pixel.x + oh < mainCamera->getHeight() && pixel.y + ow >= 0 && pixel.y + ow < mainCamera->getWidth())
                z = depthMap[pixel.x + oh][pixel.y + ow];
            if (z == -1)
            {
                switch (face)
                {
                case 0:
                    if (ow == dist)
                    {
                        face++;
                        oh++;
                    }
                    else ow++;
                    break;

                case 1:
                    if (oh == dist)
                    {
                        face++;
                        ow--;
                    }
                    else oh++;
                    break;

                case 2:
                    if (ow == -dist)
                    {
                        face++;
                        oh--;
                    }
                    else ow--;
                    break;

                case 3:
                    if (oh == -dist)
                    {
                        face++;
                        ow++;
                    }
                    else oh--;
                    break;

                case 4:
                    if (ow >= -1)
                    {
                        ow = 0;
                        oh--;
                        face = 0;
                        dist++;
                    }
                    else ow++;
                    break;
                }
            }
        }

        if (dist > maxDist || dist > qMax(mainCamera->getWidth(), mainCamera->getHeight())) point = vec4(0, 0, 0, -1);
        else
        {
            pixel.x += oh;
            pixel.y += ow;
            pixel.z = getDepth(pixel.x, pixel.y);

            pixel.w = pixel.x;
            pixel.x = pixel.y;
            pixel.y = mainCamera->getHeight() - pixel.w;
            pixel.w = 1;
            pixel.x = 2 * pixel.x / mainCamera->getWidth() - 1;
            pixel.y = 2 * pixel.y / mainCamera->getHeight() - 1;
            pixel.z = 2 * pixel.z - 1;
            if (mainCamera->getProjectionType() == Camera::EQUIRECTANGULAR)
            {
                point = mainCamera->getwMc() * mainCamera->getcMi() * pixel;
                point.x += M_PI_2;
                //point.z = pixel.z;
            }
            else
            {
                point = mainCamera->getwMc() * mainCamera->getcMi() * pixel;
            }
            point /= point.w;

            //pixel.w = pixel.x;
            //pixel.x = pixel.y;
            //pixel.y = mainCamera->getHeight() - pixel.w;
            //pixel.w = 1;
            //point = vec4(glm::unProject(vec3(pixel), mainCamera->getcMw(), mainCamera->getiMc(), vec4(0, 0, mainCamera->getWidth(), mainCamera->getHeight())), 1);
        }
        return point;
    }

    QVector<QVector<float>> Engine3D::takeDepthMeterMap()
    {
        QVector<QVector<float>> depthMeterMap = takeDepthMap();
        float zNear = mainCamera->getNearPlane();
        float zFar = mainCamera->getFarPlane();

        for (unsigned int h = 0; h < (unsigned int)mainCamera->getHeight(); h++)
        {
            for (unsigned int w = 0; w < (unsigned int)mainCamera->getWidth(); w++)
            {
                if (depthMeterMap[h][w] < 1)
                {
                    switch (mainCamera->getProjectionType())
                    {
                    case Camera::PERSPECTIVE:
                        depthMeterMap[h][w] = 2.0f * zFar * zNear / ((zFar + zNear) - (depthMeterMap[h][w] * (zFar - zNear)));
                        break;

                    case Camera::ORTHOGRAPHIC:
                        break;

                    case Camera::EQUIRECTANGULAR:
                    {
                        depthMeterMap[h][w] = depthMeterMap[h][w] * (zFar - zNear) + zNear;
                        break;
                    }

                    default:
                        break;
                    }
                }
                else
                    depthMeterMap[h][w] = -1;
            }
        }

        return depthMeterMap;
    }

    QImage Engine3D::takeDepthPicture()
    {
        QVector<QVector<float>> depthMap = takeDepthMap();

        float zMin = 1, zMax = 0;
        for (unsigned int h = 0; h < mainCamera->getHeight(); h++)
        {
            for (unsigned int w = 0; w < mainCamera->getWidth(); w++)
            {
                if (depthMap[h][w] < 1)
                {
                    zMin = qMin(zMin, depthMap[h][w]);
                    zMax = qMax(zMax, depthMap[h][w]);
                }
                else depthMap[h][w] = 0;
            }
        }

        QImage depthPicture = QImage(mainCamera->getWidth(), mainCamera->getHeight(), QImage::Format_Grayscale8);
        for (unsigned int h = 0; h < mainCamera->getHeight(); h++)
        {
            for (unsigned int w = 0; w < mainCamera->getWidth(); w++)
            {
                int value = qBound(0.0, 255.0 * (depthMap[h][w] - zMin) / (zMax - zMin), 255.0);
                depthPicture.setPixelColor(w, h, QColor(value, value, value));
            }
        }

        return depthPicture;
    }

#ifdef HAVE_ViSP
    /**
     * @brief      Asks a full depth render in a vpImage.
     *
     * @return     The image where each pixel is depth in meter.
     */
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
                    if (buffer[h * mainCamera->getWidth() + w] < 1)
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

#ifdef HAVE_OpenVR
    /**
     * @brief      Starts VR.
     *
     * @return     True if VR started successfully. False otherwise.
     */
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
                    vrCameras[i]->setBackgroundColor(mainCamera->getBackgroundColor());
                }

                vrTimer.start(1000.0f / vr.m_frequency);
                emit vrStarted();
                return true;
            }
            else return false;
        }
        else return true;
    }

    /**
     * @brief      Stops VR.
     */
    void Engine3D::stopVR()
    {
        if (vr.isActive())
        {
            static bool drawLocked = false;
            if (vrTimer.isActive() && vrTimer.thread() == QThread::currentThread()) vrTimer.stop();
            if (QThread::currentThread() != thread())
            {
                drawMutex.lock();
                drawLocked = true;
                disconnect(&vrTimer);
                QEventLoop loop;
                connect(this, SIGNAL(vrStopped()), &loop, SLOT(quit()));
                emit askStopVR();
                loop.exec();
            }
            else
            {
                if(!drawLocked) drawMutex.lock();
                vr.shutdown();
                QThreadPool::globalInstance()->setMaxThreadCount(QThreadPool::globalInstance()->maxThreadCount());

                for (unsigned int i = 0; i < 2; i++)
                {
                    delete vrCameras[i];
                    vrCameras[i] = nullptr;
                }

                delete eyesFBO;
                eyesFBO = nullptr;

                mainCamera->setActive(true);
                emit vrStopped();
                drawLocked = false;
                drawMutex.unlock();
            }
        }
    }

    /**
     * @brief      Gets the VR headset.
     *
     * @return     The VR headset.
     */
    VRheadset* Engine3D::getVRheadset()
    {
        return &vr;
    }

    /**
     * @brief      Gets the VR camera.
     *
     * @param[in]  index  The index of the camera (0-1)
     *
     * @return     The VR camera.
     */
    Camera* Engine3D::getVRCamera(unsigned int index)
    {
        return vrCameras[index];
    }

    /// <summary>
    /// Returns a FBO with eyes renders in a frame.
    /// </summary>
    /// <returns>The eyes frame.</returns>
    QOpenGLFramebufferObject* Engine3D::getEyesFBO()
    {
        return eyesFBO;
    }

    void Engine3D::setVRHandsEnabled(bool enabled)
    {
        vrHandsEnabled = enabled;
    }

    void Engine3D::setVRFloorEnabled(bool enabled)
    {
        vrFloorEnabled = enabled;
    }

    void Engine3D::lockHead(bool enabled)
    {
        static bool wasViewDistanceEnabled = viewDistanceEnabled;
        static bool wasMaxVertexLimitEnabled = maxVertexLimitEnabled;
        headLocked = enabled;
        if (headLocked)
        {
            wasViewDistanceEnabled = viewDistanceEnabled;
            wasMaxVertexLimitEnabled = maxVertexLimitEnabled;
            setViewDistanceEnabled(false);
            setMaxVertexLimitEnabled(false);
            mainCamera->setActive(true);
        }
        else
        {
            setViewDistanceEnabled(wasViewDistanceEnabled);
            setMaxVertexLimitEnabled(wasMaxVertexLimitEnabled);
            mainCamera->setActive(false);
        }
    }

    bool Engine3D::isHeadLocked() const
    {
        return headLocked;
    }
#endif

    /**
     * @brief      Sets the frame counter enabled.
     *
     * @param[in]  enabled  Indicates if enabled
     */
    void Engine3D::setFrameCounterEnabled(bool enabled)
    {
        frameCounter = enabled;
    }

    /**
     * @brief      Sets the line width (OpenGL context required).
     *
     * @param[in]  _lineWidth  The line width
     */
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

    /**
     * @brief      Sets the light on main camera (OpenGL context required).
     *
     * @param[in]  enabled  Indicates if enabled
     */
    void Engine3D::setLightOnMainCamera(bool enabled)
    {
        if (QThread::currentThread() != thread())
            emit askLightOnMainCamera(enabled);
        else
        {
            lightOnMainCamera = enabled;
            makeCurrent();
            shaders[Model3D::TRIANGLES]->bind();
            glUniform1i(glGetUniformLocation(shaders[Model3D::TRIANGLES]->programId(), "lightOnCamera"), lightOnMainCamera);
            shaders[Model3D::TRIANGLES]->release();
            doneCurrent();
            emit askUpdate();
        }
    }

    /**
     * @brief      Sets the light position (OpenGL context required).
     *
     * @param[in]  _lightPosition  The light position
     */
    void Engine3D::setLightPosition(vec3 _lightPosition)
    {
        if (QThread::currentThread() != thread())
            emit askLightPosition(_lightPosition);
        else
        {
            lightPosition = _lightPosition;
            makeCurrent();
            shaders[Model3D::TRIANGLES]->bind();
            glUniform3fv(glGetUniformLocation(shaders[Model3D::TRIANGLES]->programId(), "lightPosition"), 1, value_ptr(lightPosition));
            shaders[Model3D::TRIANGLES]->release();
            doneCurrent();
            emit askUpdate();
        }
    }

    /**
     * @brief      Sets the global illumination.
     *
     * @param[in]  globalIllumination  The global illumination
     */
    void Engine3D::setGlobalIllumination(float globalIllumination)
    {
        if (QThread::currentThread() != thread())
            emit askGlobalIllumination(globalIllumination);
        else
        {
            this->globalIllumination = globalIllumination;
            makeCurrent();
            shaders[Model3D::TRIANGLES]->bind();
            shaders[Model3D::TRIANGLES]->setUniformValue("globalIllumination", globalIllumination);
            shaders[Model3D::TRIANGLES]->release();
            doneCurrent();
            emit askUpdate();
        }
    }

    /**
     * @brief      Sets the default texture color.
     *
     * @param[in]  faceColor  The default texture color
     */
    void Engine3D::setFaceColor(vec3 faceColor)
    {
        if (QThread::currentThread() != thread())
            emit askFaceColor(faceColor);
        else
        {
            this->faceColor = faceColor;
            makeCurrent();
            shaders[Model3D::TRIANGLES]->bind();
            glUniform3fv(glGetUniformLocation(shaders[Model3D::TRIANGLES]->programId(), "faceColor"), 1, value_ptr(faceColor));
            shaders[Model3D::TRIANGLES]->release();
            doneCurrent();
            emit askUpdate();
        }
    }

    /**
     * @brief      Sets the view distance.
     *
     * @param[in]  _viewDistance  The view distance
     */
    void Engine3D::setViewDistance(double _viewDistance)
    {
        viewDistance = _viewDistance;
        nextModelsUpdate();
    }

    /**
     * @brief      Sets the view distance enabled.
     *
     * @param[in]  enabled  Indicates if enabled
     */
    void Engine3D::setViewDistanceEnabled(bool enabled)
    {
        viewDistanceEnabled = enabled;
        nextModelsUpdate();
    }

    /**
     * @brief      Sets the wait loading enabled.
     *
     * @param[in]  enabled  Indicates if enabled
     */
    void Engine3D::setWaitLoading(bool enabled)
    {
        waitLoading = enabled;
    }

    /**
     * @brief      Sets the maximum vertex limit enabled.
     *
     * @param[in]  enabled  Indicates if enabled
     */
    void Engine3D::setMaxVertexLimitEnabled(bool enabled)
    {
        maxVertexLimitEnabled = enabled;
        nextModelsUpdate();
    }

    /**
     * @brief      Sets the maximum vertex limit.
     *
     * @param[in]  _maxVertexLimit  The maximum vertex limit
     */
    void Engine3D::setMaxVertexLimit(int _maxVertexLimit)
    {
        maxVertexLimit = _maxVertexLimit * 1000000;
        nextModelsUpdate();
    }

    /**
     * @brief      Sets the maximum vertex to video random access memory enabled.
     *
     * @param[in]  enabled  Indicates if enabled
     */
    void Engine3D::setMaxVertexToVRAMEnabled(bool enabled)
    {
        maxVertexToVRAMEnabled = enabled;
    }

    /**
     * @brief      Sets the maximum vertex to video random access memory.
     *
     * @param[in]  _maxVertexToVRAM  The maximum vertex to video random access memory
     */
    void Engine3D::setMaxVertexToVRAM(double _maxVertexToVRAM)
    {
        maxVertexToVRAM = _maxVertexToVRAM * 1000000;
    }

    /**
     * @brief      Sets the maximum moving depth.
     *
     * @param[in]  _maxMovingDepth  The maximum moving depth
     */
    void Engine3D::setMaxMovingDepth(int _maxMovingDepth)
    {
        maxMovingDepth = _maxMovingDepth;
    }

    /**
     * @brief      Indicates if the main camera is moving.
     *
     * @param[in]  _isMoving  Indicates if main camera moving
     */
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

    /**
     * @brief      Sort models by depth and distance
     *
     * @param      modelsByDepthAndDistance  The models sorted by depth and distance
     * @param      modelsToUnload            The models to unload
     */
    void Engine3D::sortModelsByDepthAndDistance(QHash<unsigned int, QMap<float, QList<Model3D*>>>& modelsByDepthAndDistance, QList<Model3D*>& modelsToUnload)
    {
        QMap<unsigned int, QMap<float, QList<Model3D*>>> totalModelsByDepthAndDistance;
        for (Model3D* model : models)
        {
            float minDist = cameras[0]->distanceWith(model);
            for (Camera* camera : cameras) minDist = (camera->distanceWith(model) < minDist ? camera->distanceWith(model) : minDist);
            totalModelsByDepthAndDistance[0][minDist].append(model);

            Octree* octree = dynamic_cast<Octree*>(model);
            if (octree)
            {
                for (unsigned int i = 1; i <= octree->getMaxDepth(); i++)
                {
                    for (Octree* child : octree->getDepthChildren(i))
                    {
                        minDist = cameras[0]->distanceWith(child);
                        for (Camera* camera : cameras) minDist = (camera->distanceWith(child) < minDist ? camera->distanceWith(child) : minDist);
                        totalModelsByDepthAndDistance[child->getDepth()][minDist].append(child);
                        if (breakModelsUpdater) break;
                    }
                    if (breakModelsUpdater) break;
                }
            }
            if (breakModelsUpdater) break;
        }

        for (unsigned int depth : totalModelsByDepthAndDistance.keys())
        {
            for (float dist : totalModelsByDepthAndDistance[depth].keys())
            {
                QList<Model3D*>& modelsToTest = totalModelsByDepthAndDistance[depth][dist];
                for (Model3D* model : modelsToTest)
                {
                    if (isOnScreen(model)) modelsByDepthAndDistance[depth][dist].append(model);
                    else if (model->isLiveLoading()) modelsToUnload.append(model);
                    if (breakModelsUpdater) break;
                }
                if (breakModelsUpdater) break;
            }
            if (breakModelsUpdater) break;
        }
    }

    /**
     * @brief      Update if models are visible and load or unload in consequence.
     */
    void Engine3D::updateModels()
    {
        if (modelsUpdaterMutex.tryLock())
        {
            currentVertexNumber = 0;
            QList<Model3D*> modelsToUnload;
            QHash<unsigned int, QMap<float, QList<Model3D*>>> modelsByDepthAndDistance;
            sortModelsByDepthAndDistance(modelsByDepthAndDistance, modelsToUnload);

            QFuture<void> modelsUnloader = QtConcurrent::map(modelsToUnload, std::bind(&Model3D::unloadRAM, std::placeholders::_1, waitLoading));
            QList<Model3D*> modelsToLoad;
            for (unsigned int depth : modelsByDepthAndDistance.keys())
            {
                for (float dist : modelsByDepthAndDistance[depth].keys())
                {
                    modelsToLoad.append(modelsByDepthAndDistance[depth][dist]);
                    if (breakModelsUpdater) break;
                }
                if (breakModelsUpdater) break;
            }
            QtConcurrent::blockingMap(modelsToLoad, std::bind(&Model3D::loadRAM, std::placeholders::_1, waitLoading));
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

    /**
     * @brief      Makes OpenGL context current if available.
     */
    void Engine3D::makeCurrent()
    {
        if (context)
        {
            contextMutex.lock();
            context->makeCurrent(&surface);
        }
    }

    /**
     * @brief      Release OpenGL context if available.
     */
    void Engine3D::doneCurrent()
    {
        if (context)
        {
            context->doneCurrent();
            contextMutex.unlock();
        }
    }

    /**
     * @brief      Sets the render asked.
     *
     * @param[in]  value  The value
     */
    void Engine3D::setRenderAsked(bool value)
    {
        renderAskedMutex.lock();
        renderAsked = value;
        renderAskedMutex.unlock();
    }

    /**
     * @brief      Gets the render asked.
     *
     * @return     The render asked.
     */
    bool Engine3D::getRenderAsked()
    {
        renderAskedMutex.lock();
        bool value = renderAsked;
        renderAskedMutex.unlock();
        return value;
    }

    /**
     * @brief      Renders one frame.
     */
    void Engine3D::render()
    {
        if (isInitialized())
        {
            if (drawMutex.tryLock())
            {
                makeCurrent();
                if (waitLoading)
                {
                    if (modelsUpdater.isRunning()) modelsUpdater.waitForFinished();
                    updateModels();
                }

                static unsigned long long frameNumber = 0;
                unsigned long long vertexToVRAM = 0;

                for (Camera* camera : cameras)
                {
                    if (camera->bind())
                    {
                        glViewport(0, 0, camera->getWidth(), camera->getHeight());
                        glClearColor(
                            camera->getBackgroundColor().redF(),
                            camera->getBackgroundColor().greenF(),
                            camera->getBackgroundColor().blueF(),
                            camera->getBackgroundColor().alphaF());
                        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                        
                        for (QOpenGLShaderProgram* shader : shaders)
                        {
                            shader->bind();
                            shader->setUniformValue("equirectangular", camera->getProjectionType() == Camera::EQUIRECTANGULAR);
                            glUniformMatrix4fv(glGetUniformLocation(shader->programId(), "cMw"), 1, GL_FALSE, value_ptr(camera->getcMw()));
                            glUniformMatrix4fv(glGetUniformLocation(shader->programId(), "wMc"), 1, GL_FALSE, value_ptr(camera->getwMc()));
                            glUniformMatrix4fv(glGetUniformLocation(shader->programId(), "iMc"), 1, GL_FALSE, value_ptr(camera->getiMc()));
                            shader->release();
                        }

                        for (Model3D* model : models)
                        {
                            if (model->isOnRAM())
                            {
                                if (model->isOnVRAM()) model->draw();
                                else if (!model->isOnVRAM() && vertexToVRAM <= maxVertexToVRAM || !maxVertexToVRAMEnabled || waitLoading)
                                {
                                    vertexToVRAM += model->getVertexNumber();
                                    model->loadVRAM(waitLoading);
                                    model->draw();
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
                                            if (child->isOnVRAM()) child->draw();
                                            else if (!child->isOnVRAM() && vertexToVRAM <= maxVertexToVRAM || !maxVertexToVRAMEnabled || waitLoading)
                                            {
                                                vertexToVRAM += child->getVertexNumber();
                                                child->loadVRAM(waitLoading);
                                                child->draw();
                                            }
                                            else emit askUpdate();
                                        }
                                        else child->unloadVRAM(waitLoading);
                                    }
                                }
                            }
                        }

                        if (boxShader->bind())
                        {
                            glUniformMatrix4fv(glGetUniformLocation(boxShader->programId(), "cMw"), 1, GL_FALSE, value_ptr(camera->getcMw()));
                            glUniformMatrix4fv(glGetUniformLocation(boxShader->programId(), "wMc"), 1, GL_FALSE, value_ptr(camera->getwMc()));
                            glUniformMatrix4fv(glGetUniformLocation(boxShader->programId(), "iMc"), 1, GL_FALSE, value_ptr(camera->getiMc()));
                            boxShader->setUniformValue("equirectangular", camera->getProjectionType() == Camera::EQUIRECTANGULAR);
                            boxShader->release();
                        }
                        else qDebug() << "Can't bind box shader.";

                        for (Model3D* model : models)
                        {
                            if (camera->isModelVisible(model)) model->drawBox();
                            Octree* octree = dynamic_cast<Octree*>(model);
                            if (octree)
                                for (unsigned int i = 1; i <= octree->getMaxDepth(); i++)
                                    for (Octree* child : octree->getDepthChildren(i))
                                        child->drawBox();
                        }
                        camera->release();
                    }
                }

#ifdef HAVE_OpenVR
                if (vr.isActive())
                {
                    vr.getEyeTransformations();
                    if (!headLocked)
                    {
                        mainCamera->setRotation(vr.m_mat4eyePosLeft);
                        vrCameras[0]->setPosition(vec4(mainCamera->getPosition(), 1.0f) + vr.m_mat4eyePosLeft[3]);
                        vrCameras[1]->setPosition(vec4(mainCamera->getPosition(), 1.0f) + vr.m_mat4eyePosRight[3]);
                        vrCameras[0]->setRotation(vr.m_mat4eyePosLeft);
                        vrCameras[1]->setRotation(vr.m_mat4eyePosRight);
                    }

                    for (unsigned int i = 0 ; i < 2 ; i++)
                    {
                        if (vrCameras[i]->bind())
                        {
                            glViewport(0, 0, vrCameras[i]->getWidth(), vrCameras[i]->getHeight());
                            glClearColor(
                                vrCameras[i]->getBackgroundColor().redF(),
                                vrCameras[i]->getBackgroundColor().greenF(),
                                vrCameras[i]->getBackgroundColor().blueF(),
                                vrCameras[i]->getBackgroundColor().alphaF());
                            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                            for (QOpenGLShaderProgram* shader : shaders)
                            {
                                shader->bind();
                                shader->setUniformValue("equirectangular", vrCameras[i]->getProjectionType() == Camera::EQUIRECTANGULAR);
                                glUniformMatrix4fv(glGetUniformLocation(shader->programId(), "cMw"), 1, GL_FALSE, value_ptr(vrCameras[i]->getcMw()));
                                glUniformMatrix4fv(glGetUniformLocation(shader->programId(), "wMc"), 1, GL_FALSE, value_ptr(vrCameras[i]->getwMc()));
                                glUniformMatrix4fv(glGetUniformLocation(shader->programId(), "iMc"), 1, GL_FALSE, value_ptr(vrCameras[i]->getiMc()));
                                shader->release();
                            }

                            for (Model3D* model : models)
                            {
                                if (model->isOnRAM())
                                {
                                    if (model->isOnVRAM()) model->draw();
                                    else if (!model->isOnVRAM() && vertexToVRAM <= maxVertexToVRAM || !maxVertexToVRAMEnabled || waitLoading)
                                    {
                                        vertexToVRAM += model->getVertexNumber();
                                        model->loadVRAM(waitLoading);
                                        model->draw();
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
                                                if (child->isOnVRAM()) child->draw();
                                                else if (!child->isOnVRAM() && vertexToVRAM <= maxVertexToVRAM || !maxVertexToVRAMEnabled || waitLoading)
                                                {
                                                    vertexToVRAM += child->getVertexNumber();
                                                    child->loadVRAM(waitLoading);
                                                    child->draw();
                                                }
                                                else emit askUpdate();
                                            }
                                            else child->unloadVRAM(waitLoading);
                                        }
                                    }
                                }
                            }

                            if (boxShader->bind())
                            {
                                glUniformMatrix4fv(glGetUniformLocation(boxShader->programId(), "cMw"), 1, GL_FALSE, value_ptr(vrCameras[i]->getcMw()));
                                glUniformMatrix4fv(glGetUniformLocation(boxShader->programId(), "wMc"), 1, GL_FALSE, value_ptr(vrCameras[i]->getwMc()));
                                glUniformMatrix4fv(glGetUniformLocation(boxShader->programId(), "iMc"), 1, GL_FALSE, value_ptr(vrCameras[i]->getiMc()));
                                boxShader->release();
                            }
                            else qDebug() << "Can't bind box shader.";

                            for (Model3D* model : models)
                            {
                                if (vrCameras[i]->isModelVisible(model)) model->drawBox();
                                Octree* octree = dynamic_cast<Octree*>(model);
                                if (octree)
                                {
                                    for (unsigned int i = 1; i <= octree->getMaxDepth(); i++)
                                    {
                                        for (Octree* child : octree->getDepthChildren(i))
                                        {
                                            child->drawBox();
                                        }
                                    }
                                }
                            }

                            vr.getHandTransformations();
                            if (vrHandsEnabled)
                            {
                                for (unsigned int i = 0; i < 2; i++)
                                {
                                    if (hands[i]->isPrepared())
                                    {
                                        mat4 hp = i == 0 ? vr.m_mat4handPoseLeft : vr.m_mat4handPoseRight;
                                        hands[i]->setPosition(vec4(mainCamera->getPosition(), 1.0f) + hp[3]);
                                        hands[i]->setRotation(hp);
                                        if (!hands[i]->isOnRAM()) hands[i]->loadRAM();
                                        if (!hands[i]->isOnVRAM()) hands[i]->loadVRAM();
                                        hands[i]->draw();
                                        hands[i]->drawBox();
                                    }
                                }
                            }
                            if (vrFloorEnabled)
                            {
                                if (floor->isPrepared())
                                {
                                    floor->setPosition(vec4(mainCamera->getPosition(), 1.0f));
                                    if (!floor->isOnRAM()) floor->loadRAM();
                                    if (!floor->isOnVRAM()) floor->loadVRAM();
                                    floor->draw();
                                    floor->drawBox();
                                }
                            }
                            vrCameras[i]->release();
                            
                            if (eyesFBO)
                            {
                                if (eyesFBO->width() != vr.getWidth() * 2 || eyesFBO->height() != vr.getHeight())
                                {
                                    delete eyesFBO;
                                    eyesFBO = nullptr;
                                }
                            }
                            if (!eyesFBO)
                            {
                                eyesFBO = new QOpenGLFramebufferObject(vr.getWidth() * 2, vr.getHeight());
                                eyesFBO->release();
                            }
                            QOpenGLFramebufferObject::blitFramebuffer(
                                eyesFBO,
                                QRect(i * eyesFBO->width() / 2, 0, eyesFBO->width() / 2, eyesFBO->height()),
                                vrCameras[i]->getFBO(),
                                QRect(0, 0, vrCameras[i]->getWidth(), vrCameras[i]->getHeight()),
                                GL_COLOR_BUFFER_BIT,
                                GL_LINEAR
                            );

                            vr::Texture_t eyeTexture = { (void*)(uintptr_t)vrCameras[i]->texture(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
                            vr::VRCompositor()->Submit(i ? vr::Eye_Right : vr::Eye_Left, & eyeTexture);
                        }
                    }
                }
                //connect(&vrTimer, SIGNAL(timeout()), this, SLOT(update()));
#endif
                setFrame();

                if (frameCounter) qDebug() << ++frameNumber;

                GLenum err;
                while ((err = glGetError()) != GL_NO_ERROR) qDebug() << err;

                doneCurrent();
                drawMutex.unlock();
            }
            else updateNextAsked = true;
        }
        else updateNextAsked = true;

        setRenderAsked(false);
        if (updateNextAsked)
        {
            updateNextAsked = false;
            emit askUpdate();
        }
    }

    /**
     * @brief      Ask for a new models update. If a current update is processing, waits end.
     */
    void Engine3D::nextModelsUpdate()
    {
        if (!modelsUpdater.isRunning())
            modelsUpdater = QtConcurrent::run(&Engine3D::updateModels, this);
        else
            updateModelsNextAsked = true;
    }

    /**
     * @brief      Sets the frame (private).
     */
    void Engine3D::setFrame()
    {
#ifdef HAVE_OpenVR
        if (vr.isActive())
        {
            frameMutex.lock();
            frame = eyesFBO->toImage();
            frameMutex.unlock();
            emit frameReady(frame);
        }
        else
        {
#endif
            if (mainCamera && mainCamera->isActive())
            {
                frameMutex.lock();
                frame = mainCamera->toImage();
                frameMutex.unlock();
                emit frameReady(frame);
            }

            // Capture the secondary cameras while the OpenGL context is still
            // current (see render()), so Camera::getFrame() can be used to grab
            // their stream afterwards without a current context (issue #2).
            for (Camera* cam : cameras)
            {
                if (cam != mainCamera && cam->isActive())
                    cam->toImage();
            }
#ifdef HAVE_OpenVR
        }
#endif
    }

    /**
     * @brief      Destroys the object. Last call for Engine3D.
     */
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
#ifdef HAVE_OpenVR
            if (vr.isActive()) stopVR();
            for(ModelOBJ* hand : hands)
            {
                if (hand)
                {
                    delete hand;
                    hand = nullptr;
                }
            }
            if (floor)
            {
                delete floor;
                floor = nullptr;
            }
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
