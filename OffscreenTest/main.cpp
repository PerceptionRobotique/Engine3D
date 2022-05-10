#include <iostream>

#include <QGuiApplication>
#include <QOpenGLContext>
#include <QScreen>
#include <QOffscreenSurface>

#include <QProcess>

#include <Engine3D.h>
#include <visp/vpImageIo.h>

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

    QOpenGLContext context;
    QOffscreenSurface surface;
    surface.create();
    context.create();
    context.makeCurrent(&surface);

    Engine3D engine(Engine3D::NONE);

    QEventLoop loop;
    QObject::connect(&engine, SIGNAL(initializationFinished()), &loop, SLOT(quit()));
    engine.initialize();
    if(engine.getRenderMode() == Engine3D::THREADED) loop.exec();

    //engine.setFrameCounterEnabled(true);
    engine.setViewDistanceEnabled(false);
    engine.setMaxVertexLimitEnabled(false);
    engine.getMainCamera()->setSamples(8);

    engine.openModel(diskRoot + "/Users/nvill/3D Objects/ECathedrale/Other_Models/IntTout50/IntTout50.oct");
    engine.getMainCamera()->translate(vec3(0, 15, 0));
    engine.getMainCamera()->setProjectionType(Camera::EQUIRECTANGULAR);

    engine.takePicture().save("test.png");
    vpImageIo::writePFM(engine.takePFM(), "test.pfm");

    engine.destroy();
    context.doneCurrent();
    return 0;
}
