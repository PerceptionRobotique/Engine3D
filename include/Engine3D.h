#ifndef ENGINE3D_H
#define ENGINE3D_H

#include "Engine3D_global.h"

#include <QVector>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QFileInfo>
#include <QMessageBox>
#include <QMap>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>

#include "Camera.h"
#include "CameraController.h"
#include "ModelPTS.h"
#include "ModelBIN.h"
#include "Octree.h"

//#define FRAME_COUNTER

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
    int getMaxSamples();
    Model3D* getModel(unsigned int index);
    int getModelIndex(const Model3D* _model) const;
    QVector<Model3D*>& getModels();
    float getOpacity() const;

    void lockDraw();
    void unlockDraw();
    void lockModelsUpdater();
    void unlockModelsUpdater();

public slots:
    void openModel(QString fileName);
    void closeModel(unsigned int index);
    void closeModel(Model3D* model);
    void update();

    void setPointSizeEnabled(bool enabled);
    void setPointSize(double _pointSize);
    void setLineWidth(float _lineWidth);
    void setOpacityEnabled(bool enabled);
    void setOpacity(float _opacity);
    void setBlendFunction(BlendFunction _blendFunction);

    //Optimization
    void setViewDistance(double _viewDistance);
    void setViewDistanceEnabled(bool enabled);
    void setWaitLoading(bool enabled);
    void setMaxVertexLimitEnabled(bool enabled);
    void setMaxVertexLimit(int _maxVertexLimit);

private:
    mat4 offset;

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
    bool maxVertexLimitEnabled;
    unsigned long long maxVertexLimit;
    unsigned long long maxVertexToVRAM;

    QMutex drawMutex;
    QMutex modelsUpdaterMutex;
    QFuture<void> modelsUpdater;
    void sortModelsByDepthAndDistance(QHash<unsigned int, QMap<float, Model3D*>>& modelsByDepthAndDistance, QList<Model3D*>& modelsToUnload);
    void updateModels();

signals:
    void askUpdate();
};

#endif // ENGINE3D_H
