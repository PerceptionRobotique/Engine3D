#include <QGuiApplication>
#include <Engine3D.h>
#include <TrajectoryManager.h>
#include <VideoWriter.h>
#include <iostream>

using namespace MIS;
using namespace std;

int main(int argc, char* argv[])
{
	QGuiApplication app(argc, argv);
	Q_UNUSED(app);

	QSettings settings("settings.ini", QSettings::IniFormat);

	Engine3D engine(Engine3D::DIRECT);
	engine.initialize();

	Camera* camera = engine.getMainCamera();

	engine.openModel("../Model/Suzanne.obj");
	Model3D* suzanne = engine.getModel(0);

	camera->translate(vec3(0, 0, 5));

	// TRAJECTORY //
	TrajectoryManager tm;
	tm.setFPS(60);
	tm.addPose(*camera);

	camera->translate(vec3(0, 3, 0));
	camera->lookAt(suzanne);
	tm.addPose(*camera);

	camera->translate(vec3(4, 0, 0));
	camera->lookAt(suzanne);
	tm.addPose(*camera);
	
	camera->setPosition(vec3(0, 0, -5));
	camera->lookAt(suzanne);
	tm.addPose(*camera);
	tm.getPose(3)->delay = 3000;
	tm.getPose(3)->useYawPitchRoll = true;
	
	QDir("videoFrames").removeRecursively();
	QDir().mkdir("videoFrames");
	for (unsigned int frame = 0; frame < tm.getTotalFrameNumber(); frame++)
	{
		camera->setPose(tm.getFrame(frame).pose);
		engine.takePicture().save("videoFrames/" + QString::number(frame) + ".png");
		cout << frame + 1 << "/" << tm.getTotalFrameNumber() << endl;
	}

	// VIDEO WRITE //
	VideoWriter vw;
	vw.setFPS(60);
	vw.setVideoFileName("video.mp4");
	vw.setFramesPath("videoFrames");
	cout << "Ecriture de la video..." << endl;
	vw.writeVideo();
	vw.waitForFinished();

	engine.destroy();
	return 0;
}