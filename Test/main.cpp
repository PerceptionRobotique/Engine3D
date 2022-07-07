#include <QGuiApplication>
#include <Engine3D.h>

using namespace MIS;

int main(int argc, char* argv[])
{
	QGuiApplication app(argc, argv);
	Q_UNUSED(app);

	Engine3D engine(Engine3D::DIRECT);
	engine.initialize();

	QString diskRoot;
#ifdef _WIN32
	diskRoot = "C:/";
#elif __linux__
	diskRoot = "/mnt/c/";
#endif

	//engine.openModel(diskRoot + "/Users/nvill/3D Objects/ECathedrale/Other_Models/IntTout50/IntTout50.oct");
	engine.getMainCamera()->setBackgroundColor(Qt::gray);
	//engine.openModel(diskRoot + "/Users/nvill/3D Objects/Suzanne/Suzanne.obj");
	engine.openModel(diskRoot + "/Users/nvill/3D Objects/SuzanneMulti/SuzanneMulti.obj");
	engine.getMainCamera()->translate(vec3(-1.5, 0.3, 2.3));
	engine.getMainCamera()->lookAt(engine.getModel(0));
	//engine.getModel(0)->setBoxVisible(true);
	engine.takePicture().save("test.png");

	engine.destroy();
	return 0;
}