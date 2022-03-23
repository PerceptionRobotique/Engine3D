#include "Engine3D.h"

Engine3D::Engine3D(QObject* parent)
    : QObject(parent)
    , pointSize(1.0f)
    , lineWidth(3.0f)
    , opacityEnabled(false)
    , opacity(1.0f)
    , blendFunction(BLEND_1)
    , strict(false)
    , viewDistance(150.0f)
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
    delete models[index];
    models.removeAt(index);
}

void Engine3D::update()
{
    if (!modelsUpdater)
    {
        modelsUpdater = QThread::create(&Engine3D::updateModels, this);
        connect(modelsUpdater, SIGNAL(finished()), this, SLOT(modelsUpdaterFinished()));
        modelsUpdater->start();
        if (strict) modelsUpdater->wait();
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

void Engine3D::updateModels()
{
    for (Model3D* model : models)
    {
        bool modelOnScreen = false;
        for (Camera* camera : cameras)
        {
            modelOnScreen |= camera->cullingTest(model);
        }
        model->setOnScreen(modelOnScreen);

        Octree* octree = dynamic_cast<Octree*>(model);
        if (octree)
        {
            for (unsigned int i = 1; i <= octree->getMaxDepth(); i++)
            {
                for (Octree* child : octree->getDepthChildren(i))
                {
                    bool childOnScreen = false;
                    for (Camera* camera : cameras)
                    {
                        if(child->getDepth() <= round((float)child->getMaxDepth() * (1 - camera->distanceWith(child) / viewDistance)))
                            childOnScreen |= camera->cullingTest(child);
                    }
                    child->setOnScreen(childOnScreen);
                }
            }
        }
    }
}

void Engine3D::modelsUpdaterFinished()
{
    delete modelsUpdater;
    modelsUpdater = nullptr;
}