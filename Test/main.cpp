#include <QGuiApplication>
#include <Engine3D.h>
#include <TrajectoryManager.h>
#include <VideoWriter.h>

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

	QString modelName = "IntTout50";
	QString modelType = "oct";

	engine.openModel(diskRoot + "Users/nvill/3D Objects/ECathedrale/Other_Models/" + modelName + "/" + modelName + "." + modelType);

	TrajectoryManager tm;
	tm.addPose(*engine.getMainCamera());
	engine.getMainCamera()->translate(vec3(0, 15, 0));
	tm.addPose(*engine.getMainCamera(), 3000);

	if(QDir("frames").exists()) QDir("frames").removeRecursively();
	QDir().mkdir("frames");
	for (unsigned int frame = 0; frame <= tm.getTotalFrameNumber(); frame++)
	{
		engine.getMainCamera()->setPose(tm.getFrame(frame).pose);
		engine.takePicture().save("frames/" + QString::number(frame) + ".png");
		qDebug() << frame << "/" << tm.getTotalFrameNumber();
	}

	VideoWriter vw;
	vw.setFramesPath("frames");
	vw.writeVideo();
	vw.waitForFinished();

	engine.destroy();
	return 0;
}