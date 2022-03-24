#include "Engine3D.h"

Engine3D::Engine3D(QObject* parent)
    : QObject(parent)
    , boxShader(nullptr)
    , cameras(1, new Camera)
    , mainCamera(cameras.first())
    , pointSize(1.0f)
    , lineWidth(3.0f)
    , opacityEnabled(false)
    , opacity(1.0f)
    , blendFunction(BLEND_1)
    , viewDistance(150.0f)
    , viewDistanceEnabled(true)
    , waitLoading(false)
    , maxDepth(-1)
    , modelsUpdater(nullptr)
{
    cameras.append(new Camera);
    mainCamera = cameras.last();
}

Engine3D::~Engine3D()
{
    if (modelsUpdater) modelsUpdater->wait();
    for (Camera* camera : cameras) delete camera;
    for (QOpenGLShaderProgram* shader : shaders) delete shader;
    delete boxShader;
    for (Model3D* model : models) delete model;
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
}

bool Engine3D::isOnCamera(const Model3D* model, const Camera* camera) const
{
    bool onScreen = camera->cullingTest(model);
    const Octree* octree = dynamic_cast<const Octree*>(model);
    if (octree)
    {
        if(maxDepth != -1) onScreen &= octree->getDepth() <= maxDepth;
        if (viewDistanceEnabled)
        {
            if (octree->getDepth() > round((float)octree->getMaxDepth() * (1 - camera->distanceWith(octree) / viewDistance)))
                onScreen = false;
        }
    }
    return onScreen;
}

bool Engine3D::isOnScreen(const Model3D* model) const
{
    bool onScreen = false;
    for (Camera* camera : cameras)
    {
        onScreen |= isOnCamera(model, camera);
    }
    return onScreen;
}

Camera* Engine3D::getMainCamera()
{
    return mainCamera;
}

QVector<Camera*>& Engine3D::getCameras()
{
    return cameras;
}

Model3D* Engine3D::getModel(unsigned int index)
{
    return models[index];
}

QVector<Model3D*>& Engine3D::getModels()
{
    return models;
}

void Engine3D::openModel(QString fileName)
{
    QFileInfo fileInfo(fileName);
    if (fileInfo.suffix() == "pts")                                     models.append(new ModelPTS(fileName));
    else if (fileInfo.suffix() == "bin" || fileInfo.suffix() == "bini") models.append(new ModelBIN(fileName));
    else if (fileInfo.suffix() == "oct" || fileInfo.suffix() == "octi") models.append(new Octree(nullptr, fileName));

    connect(models.last(), SIGNAL(modelLoaded()), this, SIGNAL(askUpdate()));
    connect(models.last(), SIGNAL(modelChanged()), this, SIGNAL(askUpdate()));
    connect(models.last(), SIGNAL(modelLoadingDelayed()), this, SIGNAL(askUpdate()));
    connect(models.last(), SIGNAL(modelDestroyed()), this, SIGNAL(askUpdate()));
}

void Engine3D::closeModel(unsigned int index)
{
    if (modelsUpdater) modelsUpdater->wait();
    delete models[index];
    models.removeAt(index);
}

void Engine3D::update()
{
    if (!waitLoading)
    {
        if (!modelsUpdater)
        {
            modelsUpdater = QThread::create(&Engine3D::updateModels, this);
            connect(modelsUpdater, SIGNAL(finished()), this, SLOT(modelsUpdaterFinished()));
            modelsUpdater->start();
        }
        else emit askUpdate();
    }
    else
    {
        if (modelsUpdater) modelsUpdater->wait();
        updateModels();

        for (Model3D* model : models)
        {
            model->waitRAMloading();
            Octree* octree = dynamic_cast<Octree*>(model);
            if (octree)
            {
                for (Octree* child : octree->getAllChildren())
                {
                    child->waitRAMloading();
                }
            }
        }
    }

    static unsigned long long frameNumber = 0;

    for (Camera* camera : cameras)
    {
        if (camera->bind())
        {
            if (shaders[Model3D::POINTS]->bind())
            {
                //glViewport(0, 0, camera->getWidth(), camera->getHeight());
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
                    model->draw(shaders[Model3D::POINTS]);
                    Octree* octree = dynamic_cast<Octree*>(model);
                    if (octree)
                    {
                        for (unsigned int i = 1; i <= octree->getMaxDepth(); i++)
                        {
                            for (Octree* child : octree->getDepthChildren(i))
                            {
                                child->draw(shaders[Model3D::POINTS]);
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
        else qDebug() << "Can't bind camera.";
    }

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) qDebug() << err;

    qDebug() << ++frameNumber;
}

void Engine3D::setPointSizeEnabled(bool enabled)
{
    if (enabled) glEnable(GL_PROGRAM_POINT_SIZE);
    else glDisable(GL_PROGRAM_POINT_SIZE);
}

void Engine3D::setPointSize(float _pointSize)
{
    pointSize = _pointSize;
    shaders[Model3D::POINTS]->bind();
    shaders[Model3D::POINTS]->setUniformValue("pointSize", pointSize);
    shaders[Model3D::POINTS]->release();
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
}

void Engine3D::setOpacity(float _opacity)
{
    opacity = _opacity;
    shaders[Model3D::POINTS]->bind();
    shaders[Model3D::POINTS]->setUniformValue("opacity", opacity);
    shaders[Model3D::POINTS]->release();
}

void Engine3D::setBlendFunction(BlendFunction _blendFunction)
{
    blendFunction = _blendFunction;
    glBlendFunc(GL_SRC_ALPHA, blendFunction);
}

void Engine3D::setViewDistance(double _viewDistance)
{
    viewDistance = _viewDistance;
    emit askUpdate();
}

void Engine3D::setViewDistanceEnabled(bool enabled)
{
    viewDistanceEnabled = enabled;
    emit askUpdate();
}

void Engine3D::setWaitLoading(bool enabled)
{
    waitLoading = enabled;
    emit askUpdate();
}

void Engine3D::updateModels()
{
    QMap<float, Model3D*> modelsByDistance;
    for (Model3D* model : models)
    {
        float minDist = cameras[0]->distanceWith(model);
        for (Camera* camera : cameras) minDist = (camera->distanceWith(model) < minDist ? camera->distanceWith(model) : minDist);
        modelsByDistance[minDist] = model;
        Octree* octree = dynamic_cast<Octree*>(model);
        if (octree)
        {
            for (unsigned int i = 1; i <= octree->getMaxDepth(); i++)
            {
                for (Octree* child : octree->getDepthChildren(i))
                {
                    minDist = cameras[0]->distanceWith(child);
                    for (Camera* camera : cameras) minDist = (camera->distanceWith(child) < minDist ? camera->distanceWith(child) : minDist);
                    modelsByDistance[minDist] = child;
                }
                if (QThread::currentThread()->isInterruptionRequested()) break;
            }
            if (QThread::currentThread()->isInterruptionRequested()) break;
        }
        if (QThread::currentThread()->isInterruptionRequested()) break;
    }

    for (Model3D* model : modelsByDistance)
    {
        model->setOnScreen(isOnScreen(model), waitLoading);
        if (QThread::currentThread()->isInterruptionRequested()) break;
    }

    //for (Model3D* model : models)
    //{
    //    model->setOnScreen(isOnScreen(model), waitLoading);
    //    Octree* octree = dynamic_cast<Octree*>(model);
    //    if (octree)
    //    {
    //        for (unsigned int i = 1; i <= octree->getMaxDepth(); i++)
    //        {
    //            for (Octree* child : octree->getDepthChildren(i))
    //            {
    //                child->setOnScreen(isOnScreen(child), waitLoading);
    //                if (QThread::currentThread()->isInterruptionRequested()) break;
    //            }
    //            if (QThread::currentThread()->isInterruptionRequested()) break;
    //        }
    //    }
    //    if (QThread::currentThread()->isInterruptionRequested()) break;
    //}
}

void Engine3D::modelsUpdaterFinished()
{
    delete modelsUpdater;
    modelsUpdater = nullptr;
}