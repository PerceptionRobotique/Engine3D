#ifndef ENGINE3D_H
#define ENGINE3D_H

#include "Engine3D_global.h"

#include <QVector>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QFileInfo>
#include <QMessageBox>

#include "Camera.h"
#include "CameraController.h"
#include "ModelPTS.h"

class ENGINE3D_EXPORT Engine3D : private QObject, public QOpenGLFunctions
{
    Q_OBJECT

public:
    Engine3D(QObject* parent = nullptr);
    ~Engine3D();

    void initialize();

    Camera* getCamera();
    QVector<Camera*>& getCameras();
    QVector<Model3D*>& getModels();

public slots:
    void openModel(QString fileName);
    void closeModel(unsigned int index);
    void update();

    void setDepthTestEnabled(bool enabled);

private:
    QHash<Model3D::Primitives, QOpenGLShaderProgram*> shaders;
    QVector<Camera*> cameras;
    QVector<Model3D*> models;
};

#endif // ENGINE3D_H
