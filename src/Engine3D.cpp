#include "Engine3D.h"

Engine3D::Engine3D(QObject* parent)
    : QObject(parent)
{
    cameras.append(new Camera);
}

Engine3D::~Engine3D()
{
    for (Camera* camera : cameras) delete camera;
    for (QOpenGLShaderProgram* shader : shaders) delete shader;
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

    setDepthTestEnabled(true);
}

Camera* Engine3D::getCamera()
{
    return cameras[0];
}

QVector<Camera*>& Engine3D::getCameras()
{
    return cameras;
}

QVector<Model3D*>& Engine3D::getModels()
{
    return models;
}

void Engine3D::openModel(QString fileName)
{
    QFileInfo fileInfo(fileName);
    if (fileInfo.suffix() == "pts")
    {
        models.append(new ModelPTS(fileName));
    }
}

void Engine3D::closeModel(unsigned int index)
{
    delete models[index];
    models.removeAt(index);
}

void Engine3D::update()
{
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
                    if (model->isPrepared())
                    {
                        if (camera->cullingTest(model))
                        {
                            model->loadRAM();
                            model->loadVRAM();
                            model->draw(shaders[Model3D::POINTS]);
                        }
                    }
                }
                shaders[Model3D::POINTS]->release();
            }
            else QMessageBox::warning(nullptr, "Error", "Can't bind shader.");
            camera->release();
        }
        else QMessageBox::warning(nullptr, "Error", "Can't bind camera.");
    }
}

void Engine3D::setDepthTestEnabled(bool enabled)
{
    if (enabled) glEnable(GL_DEPTH_TEST);
    else glDisable(GL_DEPTH_TEST);
}