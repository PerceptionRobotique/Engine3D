#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <Engine3D.h>
#include <ModelCustom.h>
#include <CameraController.h>

using namespace MIS;

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	Q_UNUSED(app);

	QSettings settings("settings.ini", QSettings::IniFormat);

	Engine3D engine(Engine3D::DIRECT);
	engine.initialize();

	CameraController cameraController(&engine);
	cameraController.setTranslationSensitivity(10);
	cameraController.setRotationSensitivity(10);

	QString diskRoot;
#ifdef _WIN32
	diskRoot = "C:/";
#elif __linux__
	diskRoot = "/mnt/c/";
#endif

	//QString fileName = QFileDialog::getOpenFileName(nullptr, "Ouvrir un modèle 3D", settings.value("FileName").toString(), "Modèle 3D (*.pts *.bin *.bini *.oct *.octi *.obj)");
	QString fileName = settings.value("FileName").toString();
	if (!fileName.isEmpty())
	{
		settings.setValue("FileName", fileName);
		settings.sync();
		engine.openModel(fileName);
		engine.getMainCamera()->translate(vec3(0, 0, 3));
		//engine.getMainCamera()->lookAt(engine.getModel(0));
		engine.getModels().last()->setBoxVisible(true);
		engine.setLightOnMainCamera(false);
		engine.setLightPosition(vec3(0, 0, 0));
		engine.getModel(0)->setPosition(vec3(0, 0, 0));
		engine.takePicture().save("frame.png");
	}

	engine.destroy();
	return 0;
}