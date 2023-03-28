#include <QGuiApplication>
#include <Engine3D.h>
#include <TrajectoryManager.h>
#include <VideoWriter.h>
#include <iostream>

using namespace MIS;
using namespace std;

int main(int argc, char* argv[])
{
	//Pas d'interface à afficher donc QGuiApplication suffisant
	QGuiApplication app(argc, argv);
	//app est inutilisé dans ce programme (mais bien nécessaire)
	Q_UNUSED(app);

	//Création/Ouverture du fichier de paramètres
	QSettings settings("settings.ini", QSettings::IniFormat);

	//Création du moteur en mode de rendu direct
	Engine3D engine(Engine3D::DIRECT);
	//Initialisation du moteur
	engine.initialize();

	//Récupération de la caméra principale
	Camera* camera = engine.getMainCamera();

	//Ouverture du modèle
	engine.openModel("../Model/Suzanne.obj");
	//Récupération du modèle
	Model3D* suzanne = engine.getModel(0);

	//Déplacement de la caméra de 5 mètres sur l'axe Z (mouvement de recul)
	camera->translate(vec3(0, 0, 5));

	// TRAJECTORY //
	//Création d'une instance de Trajectory Manager
	TrajectoryManager tm;
	//Définition du nombre de frames par secondes
	tm.setFPS(60);
	//Ajout de la pose courante de la caméra
	tm.addPose(*camera);

	//Déplacement de la caméra
	camera->translate(vec3(0, 3, 0));
	//Centrer la vue sur le modèle
	camera->lookAt(suzanne);
	//Ajout de la pose courante de la caméra
	tm.addPose(*camera);

	//Déplacement de la caméra
	camera->translate(vec3(4, 0, 0));
	//Centrer la vue sur le modèle
	camera->lookAt(suzanne);
	//Ajout de la pose courante de la caméra
	tm.addPose(*camera);
	
	//Déplacement de la caméra
	camera->setPosition(vec3(0, 0, -5));
	//Centrer la vue sur le modèle
	camera->lookAt(suzanne);
	//Ajout de la pose courante de la caméra
	tm.addPose(*camera);
	//Définition du temps de transition de la pose 3 à 3 secondes
	tm.getPose(3)->delay = 3000;
	//La pose 3 utilisera des rotations en Yaw Pitch Roll
	tm.getPose(3)->useYawPitchRoll = true;
	
	//Suppression des captures précédentes
	QDir("videoFrames").removeRecursively();
	//Création du dossier contenant les frames
	QDir().mkdir("videoFrames");
	//Pour chaque frame
	for (unsigned int frame = 0; frame < tm.getTotalFrameNumber(); frame++)
	{
		//On définit la pose de la caméra à la frame en cours
		camera->setPose(tm.getFrame(frame).pose);
		//Enregistrement de la frame courant dans une image
		engine.takePicture().save("videoFrames/" + QString::number(frame) + ".png");
		cout << frame + 1 << "/" << tm.getTotalFrameNumber() << endl;
	}

	// VIDEO WRITE //
	//Création d'une instance de VideoWriter
	VideoWriter vw;
	//Définition du nombre de FPS de la vidéo
	vw.setFPS(60);
	//Nom de la vidéo en sortie
	vw.setVideoFileName("video.mp4");
	//On indique l'endroit où se trouvent les frames
	vw.setFramesPath("videoFrames");
	cout << "Ecriture de la video..." << endl;
	//On écrit la vidéo et on attend que l'écriture soit terminée
	vw.writeVideo();
	vw.waitForFinished();

	//Destruction du moteur
	engine.destroy();
	return 0;
}