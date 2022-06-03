#include <QGuiApplication>
#include <Engine3D.h>

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

    Engine3D engine(Engine3D::DIRECT);
    engine.initialize();

    engine.setWaitLoading(true);

    engine.openModel(diskRoot + "/Users/nvill/3D Objects/ECathedrale/Other_Models/IntTout50/IntTout50.oct");
    engine.getMainCamera()->translate(vec3(0, 15, 0));

    engine.takePicture().save("OffscreenCapture.png");

    engine.destroy();
    return 0;
}
