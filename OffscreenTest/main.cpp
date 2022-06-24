#include <QApplication>
#include <QFileDialog>
#include <Engine3D.h>

using namespace MIS;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    Q_UNUSED(app);

    QString diskRoot;
#ifdef _WIN32
    diskRoot = "C:";
#elif __linux__
    diskRoot = "/mnt/c";
#endif

    Engine3D engine(Engine3D::DIRECT);
    engine.initialize();

    engine.getMainCamera()->setBackgroundColor(QColor(255, 255, 255));

    //QString modelName = QFileDialog::getOpenFileName(nullptr, "Ouvrir un modèle")
    //engine.openModel(diskRoot + "/Users/nvill/3D Objects/ECathedrale/Other_Models/IntTout50/IntTout50.oct");
    engine.openModel(diskRoot + "/Users/nvill/3D Objects/SuzanneTri/Suzanne.obj");
    engine.openModel(diskRoot + "/Users/nvill/3D Objects/Suzanne/Suzanne.obj");

    engine.getModel(0)->setPosition(vec3(-1, 0, 0));
    engine.getModel(0)->setRotation(vec3(0, 90, 0));

    engine.getModel(1)->setPosition(vec3(1, 0, 0));
    engine.getModel(1)->setRotation(vec3(0, -90, 0));
    //engine.openModel("/media/noel/OS/Users/nvill/3D Objects/ECathedrale/Other_Models/IntTout50/IntTout50.oct");
    engine.getMainCamera()->translate(vec3(0, 0, -5));
    engine.getMainCamera()->rotate(180, vec3(0, 1, 0));

    engine.takePicture().save("OffscreenCapture.png");

    engine.destroy();
    return 0;
}
