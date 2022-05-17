#include <iostream>

#include <QGuiApplication>
#include <QOpenGLContext>
#include <QScreen>
#include <QOffscreenSurface>

#include <QTimer>

#include <Engine3D.h>
#include <CameraController.h>

using namespace MIS;

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);

    QString diskRoot;
#ifdef _WIN32
    diskRoot = "C:";
#elif __linux__
    diskRoot = "/mnt/c";
#endif

    //QOpenGLContext context;
    //QOffscreenSurface surface;
    //surface.create();
    //context.create();
    //context.makeCurrent(&surface);

    Engine3D engine(Engine3D::DIRECT);
    CameraController cameraController(&engine);
    cameraController.setTranslationSensitivity(10);
    cameraController.setRotationSensitivity(10);

    //QEventLoop loop;
    //QObject::connect(&engine, SIGNAL(initializationFinished()), &loop, SLOT(quit()));
    engine.initialize();
    //if(engine.getRenderMode() == Engine3D::THREADED) loop.exec();

    //engine.setFrameCounterEnabled(true);
    //engine.setViewDistanceEnabled(false);
    //engine.setMaxVertexLimitEnabled(false);
    //engine.getMainCamera()->setSamples(8);
    //engine.setViewDistance(100);

    engine.openModel(diskRoot + "/Users/nvill/3D Objects/ECathedrale/Other_Models/IntTout50/IntTout50.oct");
    engine.getMainCamera()->translate(vec3(0, 15, 0));
    //engine.getMainCamera()->setProjectionType(Camera::EQUIRECTANGULAR);

    //engine.takePicture().save("test.png");
    //vpImageIo::writePFM(engine.takePFM(), "test.pfm");

    QObject::connect(&engine, SIGNAL(updateVRInputs()), &cameraController, SLOT(updateVRInputs()));
    engine.startVR();

    //cameraController.setVRInputsUpdaterEnabled(true);
    QTimer timer;
    QEventLoop loop;
    QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    timer.start(45000);
    loop.exec();

    engine.stopVR();

    engine.destroy();
    //context.doneCurrent();
    return 0;
}
