#include <iostream>

#include <QOpenGLContext>
#include <QScreen>
#include <QOffscreenSurface>

#include <Engine3D.h>

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

    Engine3D* engine = new Engine3D;
    QOpenGLContext context;
    QOffscreenSurface surface;

    surface.create();
    context.create();
    context.makeCurrent(&surface);
    engine->initialize();

    engine->setWaitLoading(true);
    engine->setViewDistanceEnabled(false);
    engine->setMaxVertexLimitEnabled(false);
    engine->openModel(diskRoot + "/Users/nvill/3D Objects/ECathedrale/Other_Models/IntTout50/IntTout50.oct");
    engine->getMainCamera()->translate(vec3(0, 15, 0));
    engine->render();
    engine->getMainCamera()->toImage().save("test.png");

    delete engine;
    context.doneCurrent();
    return 0;
}
