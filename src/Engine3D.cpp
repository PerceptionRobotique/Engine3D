#include "Engine3D.h"

Engine3D::Engine3D(QObject* parent)
    : QObject(parent)
    , initialized(false)
    , boxShader(nullptr)
    , cameras(1, new Camera)
    , mainCamera(cameras.first())
    , frameCounter(false)
    , pointSize(1.0f)
    , lineWidth(3.0f)
    , opacityEnabled(false)
    , opacity(1.0f)
    , blendFunction(BLEND_1)
    , viewDistance(150.0f)
    , viewDistanceEnabled(true)
    , waitLoading(false)
    , maxDepth(-1)
    , maxMovingDepth(-1)
    , isMoving(false)
    , maxVertexLimitEnabled(true)
    , maxVertexLimit(100000000)
    , maxVertexToVRAM(1000000)
    , updateNextAsked(false)
    , breakModelsUpdater(false)
{
#ifdef __linux__
    Q_INIT_RESOURCE(Engine3D);
#endif
    connect(mainCamera, SIGNAL(cameraChanged()), this, SLOT(nextModelsUpdate()));
}

Engine3D::~Engine3D()
{
    modelsUpdaterMutex.lock();
    for (Camera* camera : cameras) delete camera;
    for (QOpenGLShaderProgram* shader : shaders) delete shader;
    delete boxShader;
    for (Model3D* model : models) delete model;
    modelsUpdaterMutex.unlock();
}

void Engine3D::initialize()
{
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
    setPointSizeEnabled(true);
    setPointSize(pointSize);
    setLineWidth(lineWidth);
    setOpacityEnabled(opacityEnabled);
    setOpacity(opacity);
    setBlendFunction(blendFunction);

    initialized = true;
}

bool Engine3D::isInitialized() const
{
    return initialized;
}

bool Engine3D::isOnCamera(const Model3D* model, const Camera* camera) const
{
    bool onScreen = camera->cullingTest(model);
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
    return onScreen;
}

bool Engine3D::isOnScreen(const Model3D* model)
{
    if (!maxVertexLimitEnabled || currentVertexNumber + model->getVertexNumber() <= maxVertexLimit)
    {
        bool onScreen = false;
        for (Camera* camera : cameras) onScreen |= isOnCamera(model, camera);
        if(onScreen) currentVertexNumber += model->getVertexNumber();
        return onScreen;
    }
    else
        return false;
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
    int maxSamples;
    glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
    return maxSamples;
}

Model3D* Engine3D::getModel(unsigned int index)
{
    return models[index];
}

int Engine3D::getModelIndex(const Model3D* _model) const
{
    int index = -1;
    for (unsigned int i = 0 ; i < models.count() ; i++)
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

    connect(models.last(), SIGNAL(modelLoaded()), this, SLOT(nextModelsUpdate()));
    connect(models.last(), SIGNAL(modelUnloaded()), this, SLOT(nextModelsUpdate()));
    connect(models.last(), SIGNAL(modelChanged()), this, SLOT(nextModelsUpdate()));
    connect(models.last(), SIGNAL(modelLoadingDelayed()), this, SLOT(nextModelsUpdate()));
    connect(models.last(), SIGNAL(modelDestroyed()), this, SLOT(nextModelsUpdate()));
}

void Engine3D::closeModel(unsigned int index)
{
    modelsUpdaterMutex.lock();
    delete models[index];
    models.removeAt(index);
    modelsUpdaterMutex.unlock();
}

void Engine3D::closeModel(Model3D* model)
{
    int index = getModelIndex(model);
    if (index >= 0) closeModel(index);
}

void Engine3D::render()
{
    if (drawMutex.tryLock())
    {
        if (waitLoading) modelsUpdater.waitForFinished();
        if (!modelsUpdater.isRunning()) modelsUpdater = QtConcurrent::run(&Engine3D::updateModels, this);
        if (waitLoading) modelsUpdater.waitForFinished();

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
                            else if (!model->isOnVRAM() && vertexToVRAM <= maxVertexToVRAM || waitLoading)
                            {
                                vertexToVRAM += model->getVertexNumber();
                                model->loadVRAM(waitLoading);
                                model->draw(shaders[Model3D::POINTS]);
                            }
                            else update();
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
                                        else if (!child->isOnVRAM() && vertexToVRAM <= maxVertexToVRAM || waitLoading)
                                        {
                                            vertexToVRAM += child->getVertexNumber();
                                            child->loadVRAM(waitLoading);
                                            child->draw(shaders[Model3D::POINTS]);
                                        }
                                        else update();
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

        //ANALYSE
        //unsigned long long vertexOnRAM = 0;
        //unsigned long long vertexOnVRAM = 0;
        //for (Model3D* model : models)
        //{
        //    vertexOnRAM += (int)model->isOnRAM() * model->getVertexNumber();
        //    vertexOnVRAM += (int)model->isOnVRAM() * model->getVertexNumber();
        //    Octree* octree = dynamic_cast<Octree*>(model);
        //    if (octree)
        //    {
        //        for (Model3D* child : octree->getAllChildren())
        //        {
        //            vertexOnRAM += (int)child->isOnRAM() * child->getVertexNumber();
        //            vertexOnVRAM += (int)child->isOnVRAM() * child->getVertexNumber();
        //        }
        //    }
        //}
        //qDebug() << "On RAM : " << vertexOnRAM;
        //qDebug() << "On VRAM : " << vertexOnVRAM;

        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) qDebug() << err;

        if(frameCounter) qDebug() << ++frameNumber;
        drawMutex.unlock();
        emit engineUpdated();
        if (updateNextAsked)
        {
            updateNextAsked = false;
            emit askUpdate();
        }
    }
    else updateNextAsked = true;
}

void Engine3D::update()
{
    if (drawMutex.tryLock())
    {
        drawMutex.unlock();
        emit askUpdate();
    }
    else updateNextAsked = true;
}

void Engine3D::setFrameCounterEnabled(bool enabled)
{
    frameCounter = enabled;
}

void Engine3D::setPointSizeEnabled(bool enabled)
{
    if (enabled) glEnable(GL_PROGRAM_POINT_SIZE);
    else glDisable(GL_PROGRAM_POINT_SIZE);
    update();
}

void Engine3D::setPointSize(double _pointSize)
{
    pointSize = _pointSize;
    shaders[Model3D::POINTS]->bind();
    shaders[Model3D::POINTS]->setUniformValue("pointSize", pointSize);
    shaders[Model3D::POINTS]->release();
    update();
}

void Engine3D::setLineWidth(float _lineWidth)
{
    lineWidth = _lineWidth;
    glLineWidth(lineWidth);
}

void Engine3D::setOpacityEnabled(bool enabled)
{
    opacityEnabled = enabled;
    if (opacityEnabled)
    {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
    }
    else
    {
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }
    update();
}

void Engine3D::setOpacity(float _opacity)
{
    opacity = _opacity;
    shaders[Model3D::POINTS]->bind();
    shaders[Model3D::POINTS]->setUniformValue("opacity", opacity);
    shaders[Model3D::POINTS]->release();
    update();
}

void Engine3D::setBlendFunction(BlendFunction _blendFunction)
{
    blendFunction = _blendFunction;
    glBlendFunc(GL_SRC_ALPHA, blendFunction);
    update();
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
    update();
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

void Engine3D::setMaxVertexToVRAM(double _maxVertexToVRAM)
{
    maxVertexToVRAM = _maxVertexToVRAM * 1000000;
}

void Engine3D::setMaxMovingDepth(int _maxMovingDepth)
{
    maxMovingDepth = _maxMovingDepth;
    nextModelsUpdate();
}

void Engine3D::setMoving(bool _isMoving)
{
    isMoving = _isMoving;
    update();
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
    breakModelsUpdater = false;
}

void Engine3D::nextModelsUpdateThread()
{
    modelsUpdater.waitForFinished();
    update();
}

void Engine3D::nextModelsUpdate()
{
    if (!nextModelsUpdater.isRunning()) nextModelsUpdater = QtConcurrent::run(&Engine3D::nextModelsUpdateThread, this);
    else update();
}