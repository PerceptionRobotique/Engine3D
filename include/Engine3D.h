#ifndef ENGINE3D_H
#define ENGINE3D_H

#include "Engine3D_global.h"

#include <QVector>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QFileInfo>
#include <QMessageBox>
#include <QMap>

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

    bool isOnCamera(const Model3D* model, const Camera* camera) const;
    bool isOnScreen(const Model3D* model) const;

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

    //Optimization
    void setViewDistance(double _viewDistance);
    void setViewDistanceEnabled(bool enabled);
    void setWaitLoading(bool enabled);
    void setLimitMaxVertexEnabled(bool enabled);
    void setLimitMaxVertex(double _vertexMaxLimit);

private:
    QHash<Model3D::Primitives, QOpenGLShaderProgram*> shaders;
    QOpenGLShaderProgram* boxShader;
    QVector<Camera*> cameras;
    Camera* mainCamera;
    QVector<Model3D*> models;

    float pointSize;
    float lineWidth;
    bool opacityEnabled;
    float opacity;
    BlendFunction blendFunction;

    //Optimization
    float viewDistance;
    bool viewDistanceEnabled; //limit loading distance
    bool waitLoading; //wait models loading
    int maxDepth;
    int maxMovingDepth;
    bool limitMaxVertex;
    unsigned long long vertexMaxLimit;
    unsigned long long maxVertexToVRAM;

    QMutex drawMutex;
    QThread* modelsUpdater;
    void updateModels();

private slots:
    void modelsUpdaterFinished();

signals:
    void askUpdate();
};

#endif // ENGINE3D_H
