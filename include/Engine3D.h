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
#include "ModelBIN.h"
#include "Octree.h"

class ENGINE3D_EXPORT Engine3D : public QObject, public QOpenGLFunctions
{
    Q_OBJECT

public:
    enum BlendFunction {
        BLEND_1 = GL_ONE,
        BLEND_2 = GL_ONE_MINUS_SRC_ALPHA
    };

    Engine3D(QObject* parent = nullptr);
    ~Engine3D();

    void initialize();

    Camera* getMainCamera();
    QVector<Camera*>& getCameras();
    Model3D* getModel(unsigned int index);
    QVector<Model3D*>& getModels();

public slots:
    void openModel(QString fileName);
    void closeModel(unsigned int index);
    void update();

    void setPointSizeEnabled(bool enabled);
    void setPointSize(float _pointSize);
    void setLineWidth(float _lineWidth);
    void setOpacityEnabled(bool enabled);
    void setOpacity(float _opacity);
    void setBlendFunction(BlendFunction _blendFunction);

private:
    QHash<Model3D::Primitives, QOpenGLShaderProgram*> shaders;
    QOpenGLShaderProgram* boxShader;
    Camera* mainCamera;
    QVector<Camera*> cameras;
    QVector<Model3D*> models;

    float pointSize;
    float lineWidth;
    bool opacityEnabled;
    float opacity;
    BlendFunction blendFunction;
    bool strict;

    QMutex drawMutex;
    QThread* modelsUpdater;
    void updateModels();

private slots:
    void modelsUpdaterFinished();

signals:
    void askUpdate();
};

#endif // ENGINE3D_H
