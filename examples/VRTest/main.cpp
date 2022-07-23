#include <QGuiApplication>
#include <Engine3D.h>
#include <CameraController.h>

using namespace MIS;

int main(int argc, char* argv[])
{
	QGuiApplication app(argc, argv);
	Q_UNUSED(app);

	Engine3D engine(Engine3D::DIRECT);
	engine.initialize();

	engine.openModel("../Model/Suzanne.obj");
	engine.getMainCamera()->translate(vec3(0, 0, 5));

	CameraController cameraController(&engine);
	cameraController.setTranslationSensitivity(5);
	cameraController.setRotationSensitivity(5);

	engine.startVR();
	cameraController.setVRheadset(engine.getVRheadset());
	cameraController.setVRInputsUpdaterEnabled(true);

	QTimer timer;
	QEventLoop loop;
	timer.singleShot(30000, &loop, SLOT(quit()));
	loop.exec();

	engine.stopVR();

	engine.destroy();
	return 0;
}