#include <QApplication>
#include <QFileDialog>
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

	//QString fileName = QFileDialog::getOpenFileName(nullptr, "Ouvrir un modèle 3D", settings.value("FileName").toString(), "Modèle 3D (*.pts *.bin *.bini *.oct *.octi *.obj)");
	//if (!fileName.isEmpty())
	//{
	//	settings.setValue("FileName", fileName);
	//	settings.sync();
	//	engine.openModel(fileName);
		
		ModelCustom* model = new ModelCustom(Model3D::POINTS, true, engine.getShaders()[Model3D::POINTS], engine.getBoxShader());

		QVector<vec3> pos = {
			vec3(-1, -1, -1),
			vec3(0, 0, 0),
			vec3(1, 1, 1)
		};
		QVector<unsigned char> color = {
			255, 0, 0,
			0, 255, 0,
			0, 0, 255
		};

		model->addVertex(pos, color);
		model->setPointSize(10);
		
		engine.addModel(model);
		engine.getMainCamera()->translate(vec3(0, 0, 2.1));
		//engine.getMainCamera()->setProjectionType(Camera::EQUIRECTANGULAR);
		engine.takePicture().save("capture.png");
		//engine.takeDepthPicture().save("captureDepth.png");
		model->removeVertex(0);
		engine.takePicture().save("capture2.png");
	//}

	engine.destroy();
	return 0;
}