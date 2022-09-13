#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <Engine3D.h>
#include <ModelCustom.h>

using namespace MIS;

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	Q_UNUSED(app);

	QSettings settings("settings.ini", QSettings::IniFormat);

	Engine3D engine(Engine3D::DIRECT);
	engine.initialize();

	QString diskRoot;
#ifdef _WIN32
	diskRoot = "C:/";
#elif __linux__
	diskRoot = "/mnt/c/";
#endif

	QString fileName = QFileDialog::getOpenFileName(nullptr, "Ouvrir un modèle 3D", settings.value("FileName").toString(), "Modèle 3D (*.pts *.bin *.bini *.oct *.octi *.obj)");
	if (!fileName.isEmpty())
	{
		settings.setValue("FileName", fileName);
		settings.sync();
		engine.openModel(fileName);	
		QMessageBox stopBox;
		stopBox.setText("VR is running.");
		stopBox.setStandardButtons(QMessageBox::Close);
		engine.startVR();
		stopBox.exec();
		engine.stopVR();
	}

	engine.destroy();
	return 0;
}