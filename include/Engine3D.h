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

    /**
     * @brief      This class describes a 3D Engine improved for huge cloud points display using OpenGL.
     */
    class ENGINE3D_EXPORT Engine3D : public QObject, public QOpenGLFunctions
    {
        Q_OBJECT

    public:
        enum RenderMode {
            NONE = 0,
            DIRECT = 1,
            THREADED = 2
        };

        Engine3D(RenderMode _renderMode = DIRECT, QObject* parent = nullptr);
        ~Engine3D();

        bool isInitialized() const;

        bool isModelVisible(const Model3D* model) const;
        bool isOnCamera(const Model3D* model, const Camera* camera) const;
        bool isOnScreen(const Model3D* model);

        RenderMode getRenderMode() const;
        QOpenGLContext* getContext();
        QHash<Model3D::Primitives, QOpenGLShaderProgram*> getShaders();
        QOpenGLShaderProgram* getBoxShader();
        QImage getFrame();
        Camera* getMainCamera();
        QVector<Camera*>& getCameras();
        int getMaxSamples();
        Model3D* getModel(unsigned int index);
        Model3D* getModel(const QString& name);
        int getModelIndex(const Model3D* _model) const;
        QVector<Model3D*>& getModels();
        bool getWaitLoading() const;
        bool isViewDistanceEnabled() const;
        bool isMaxVertexLimitEnabled() const;
        vec3 getFaceColor() const;

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
        void addModel(Model3D* model);
        void openModel(QString fileName);
        void closeModel(unsigned int index);
        void closeModel(Model3D* model);
        void update();
        QImage takePicture();
        QVector<QVector<float>> takeDepthMap();
        float getDepth(unsigned int h, unsigned int w);
        float getDepthMeter(unsigned int h, unsigned int w);
        vec4 getNearestPoint(unsigned int h, unsigned int w, unsigned int maxDist = 20);
        QVector<QVector<float>> takeDepthMeterMap();
        QImage takeDepthPicture();
#ifdef HAVE_VISP
        vpImage<float> takePFM();
#endif
#ifdef HAVE_VR
        bool startVR();
        void stopVR();
        VRheadset* getVRheadset();
        Camera* getVRCamera(unsigned int index);
        QOpenGLFramebufferObject* getEyesFBO();
#endif
        void destroy();

        //Debug
        void setFrameCounterEnabled(bool enabled);

        //Render
        void setLineWidth(float _lineWidth);
        void setLightOnMainCamera(bool enabled);
        void setLightPosition(vec3 _lightPosition);
        void setGlobalIllumination(float globalIllumination);
        void setFaceColor(vec3 faceColor);

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
        QOpenGLFramebufferObject* eyesFBO;
        QVector<ModelOBJ*> hands;
        ModelOBJ* floor;
#endif
        QVector<Model3D*> models;

        //Debug
        bool frameCounter;

        //Render
        float lineWidth;
        bool lightOnMainCamera;
        vec3 lightPosition;
        float globalIllumination;
        vec3 faceColor;

        bool renderAsked;
        QMutex renderAskedMutex;
        QImage frame;
        QMutex frameMutex;
        QImage pictureAsked;
        float depthAsked;
        QVector<QVector<float>> depthMapAsked;
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
        void askDepth(unsigned int h, unsigned int w);
        void askDepthMap();
#ifdef HAVE_VISP
        void askPFM();
#endif
        void pictureTaken();
        void frameReady(QImage);
        void askDestroy();
        void destructionFinished();
        void askLineWidth(float);
        void askLightOnMainCamera(bool);
        void askLightPosition(vec3);
        void askGlobalIllumination(float);
        void askFaceColor(vec3);
        void renderModeChanged();
#ifdef HAVE_VR
        void vrStarted();
        void askStopVR();
        void vrStopped();
        void updateVRInputs();
#endif
    };

}

#endif // ENGINE3D_H
