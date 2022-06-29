#include <QApplication>
#include <QSettings>
#include <QFileDialog>
#include <Engine3D.h>

using namespace MIS;

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	QSettings settings("settings.ini", QSettings::IniFormat);

	Engine3D engine(Engine3D::DIRECT);
	engine.initialize();

	QString modelFileName = QFileDialog::getOpenFileName(nullptr, "Ouvrir un modèle 3D", settings.value("ModelFileName").toString(), "Modèle 3D (*.pts *.bin *.bini *.oct *.octi *.obj)");
	QString captureFileName = QFileDialog::getSaveFileName(nullptr, "Enregistrer capture", settings.value("CaptureFileName").toString(), "Fichier PNG (*.png)");
	if (!modelFileName.isEmpty() && !captureFileName.isEmpty())
	{
		settings.setValue("ModelFileName", modelFileName);
		settings.setValue("CaptureFileName", captureFileName);
		engine.openModel(modelFileName);
		engine.getMainCamera()->translate(vec3(0, 0, 5));
		engine.getMainCamera()->lookAt(engine.getModel(0));
		engine.takePicture().save(captureFileName);
	}

	engine.destroy();
	return 0;
}