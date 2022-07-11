#ifndef ENGINE3D_H
#define ENGINE3D_H

#include "Engine3D_global.h"

#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QVector>
#include <QMap>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>

#ifdef HAVE_VISP
#include <visp/vpImage.h>
#include <visp/vpImageTools.h>
#endif

#ifdef HAVE_VR
#include <VRheadset.h>
#endif

#ifdef ANDROID
#include <QtCore/private/qandroidextras_p.h>
#endif

#include "Camera.h"
#include "ModelPTS.h"
#include "ModelBIN.h"
#include "ModelOBJ.h"
#include "Octree.h"

namespace MIS
{

    class ENGINE3D_EXPORT Engine3D : public QObject, public QOpenGLFunctions
    {
        Q_OBJECT

    public:
        enum RenderMode {
            NONE = 0,
            DIRECT = 1,
            THREADED = 2
        };

        enum BlendFunction {
            BLEND_1 = GL_ONE_MINUS_SRC_ALPHA,
            BLEND_2 = GL_ONE
        };

        Engine3D(RenderMode _renderMode = DIRECT, QObject* parent = nullptr);
        ~Engine3D();

        bool isInitialized() const;

        bool isModelVisible(const Model3D* model) const;
        bool isOnCamera(const Model3D* model, const Camera* camera) const;
        bool isOnScreen(const Model3D* model);

        RenderMode getRenderMode() const;
        QOpenGLContext* getContext();
        QImage getFrame();
        Camera* getMainCamera();
        QVector<Camera*>& getCameras();
        int getMaxSamples();
        Model3D* getModel(unsigned int index);
        int getModelIndex(const Model3D* _model) const;
        QVector<Model3D*>& getModels();
        float getOpacity() const;
        bool getWaitLoading() const;

        void lockDraw();
        void unlockDraw();
        void lockModelsUpdater();
        void unlockModelsUpdater();

    public slots:
        void setRenderMode(RenderMode _renderMode);
        void initialize();
        void setMainCamera(Camera* camera);
        void addCamera(Camera* camera);
        void removeCamera(unsigned int index);
        void removeCamera(Camera* camera);
        void openModel(QString fileName);
        void closeModel(unsigned int index);
        void closeModel(Model3D* model);
        void update();
        QImage takePicture();
#ifdef HAVE_VISP
        vpImage<float> takePFM();
#endif
#ifdef HAVE_VR
        bool startVR();
        void stopVR();
        VRheadset* getVRheadset();
        Camera* getVRCamera(unsigned int index);
#endif
        void destroy();

        //Debug
        void setFrameCounterEnabled(bool enabled);

        //Render
        void setPointSizeEnabled(bool enabled);
        void setPointSize(double _pointSize);
        void setLineWidth(float _lineWidth);
        void setOpacityEnabled(bool enabled);
        void setOpacity(float _opacity);
        void setBlendFunction(BlendFunction _blendFunction);
        void setLightOnMainCamera(bool enabled);
        void setLightPosition(vec3 _lightPosition);

        //Optimization
        void setViewDistance(double _viewDistance);
        void setViewDistanceEnabled(bool enabled);
        void setWaitLoading(bool enabled);
        void setMaxVertexLimitEnabled(bool enabled);
        void setMaxVertexLimit(int _maxVertexLimit);
        void setMaxVertexToVRAMEnabled(bool enabled);
        void setMaxVertexToVRAM(double _maxVertexToVRAM);
        void setMaxMovingDepth(int _maxMovingDepth);
        void setMoving(bool _isMoving);

    private:
        RenderMode renderMode;
        bool initialized;

        QOffscreenSurface surface;
        QOpenGLContext* context;
        QMutex contextMutex;
        QThread renderThread;

        QHash<Model3D::Primitives, QOpenGLShaderProgram*> shaders;
        QOpenGLShaderProgram* boxShader;
        QVector<Camera*> cameras;
        Camera* mainCamera;
#ifdef HAVE_VR
        VRheadset vr;
        QVector<Camera*> vrCameras;
        QTimer vrTimer;
#endif
        QVector<Model3D*> models;

        //Debug
        bool frameCounter;

        //Render
        float pointSize;
        float lineWidth;
        bool opacityEnabled;
        float opacity;
        BlendFunction blendFunction;
        bool lightOnMainCamera;
        vec3 lightPosition;

        bool renderAsked;
        QMutex renderAskedMutex;
        QImage frame;
        QMutex frameMutex;
        QImage pictureAsked;
#ifdef HAVE_VISP
        vpImage<float> pfmAsked;
#endif

        //Optimization
        int maxSamples;
        float viewDistance;
        bool viewDistanceEnabled; //limit loading distance
        bool waitLoading; //wait models loading
        int maxDepth;
        int maxMovingDepth;
        int isMoving;
        bool maxVertexLimitEnabled;
        unsigned long long maxVertexLimit;
        unsigned long long currentVertexNumber;
        bool maxVertexToVRAMEnabled;
        unsigned long long maxVertexToVRAM;

        QMutex drawMutex;
        bool updateNextAsked;
        QMutex modelsUpdaterMutex;
        bool breakModelsUpdater;
        bool updateModelsNextAsked;
        QFuture<void> modelsUpdater;

        void sortModelsByDepthAndDistance(QHash<unsigned int, QMap<float, QList<Model3D*>>>& modelsByDepthAndDistance, QList<Model3D*>& modelsToUnload);
        void updateModels();
        void makeCurrent();
        void doneCurrent();
        void setRenderAsked(bool value);
        bool getRenderAsked();

    private slots:
        void render();
        void nextModelsUpdate();
        void setFrame();

    signals:
        void askRenderMode(RenderMode);
        void askInitialization();
        void initializationFinished();
        void askClose(unsigned int);
        void askRender();
        void askUpdate();
        void askPicture();
#ifdef HAVE_VISP
        void askPFM();
#endif
        void pictureTaken();
        void frameReady(QImage);
        void askDestroy();
        void destructionFinished();
        void askPointSizeEnabled(bool);
        void askPointSize(double);
        void askLineWidth(float);
        void askOpacityEnabled(bool);
        void askOpacity(float);
        void askBlendFunction(BlendFunction);
        void askLightOnMainCamera(bool);
        void askLightPosition(vec3);
        void renderModeChanged();
#ifdef HAVE_VR
        void askStopVR();
        void vrStopped();
        void updateVRInputs();
#endif
    };

}

#endif // ENGINE3D_H
