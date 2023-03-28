#include <QGuiApplication>
#include <Engine3D.h>
#include <CameraController.h>

using namespace MIS;

int main(int argc, char* argv[])
{
	//Pas d'interface à afficher donc QGuiApplication suffisant
	QGuiApplication app(argc, argv);
	//app est inutilisé dans ce programme (mais bien nécessaire)
	Q_UNUSED(app);

	//Création du moteur en mode de rendu direct
	Engine3D engine(Engine3D::DIRECT);
	//Initialisation du moteur
	engine.initialize();

	//Ouverture du modèle
	engine.openModel("../Model/Suzanne.obj");
	//Déplacement de la caméra de 3 mètres sur l'axe Z (mouvement de recul)
	engine.getMainCamera()->translate(vec3(0, 0, 5));

	//Création d'une instance de CameraController associée à engine
	CameraController cameraController(&engine);
	//Définition des sensibilités en translation et rotation
	cameraController.setTranslationSensitivity(5);
	cameraController.setRotationSensitivity(5);

	//Démarrage de la VR
	engine.startVR();
	//Lien des déplacements du casque avec la caméra d'engine au travers de cameraController
	cameraController.setVRheadset(engine.getVRheadset());
	//Activation de la lecture des entrées des contrôleurs VR
	cameraController.setVRInputsUpdaterEnabled(true);

	//On attend 30 secondes
	QTimer timer;
	QEventLoop loop;
	timer.singleShot(30000, &loop, SLOT(quit()));
	loop.exec();

	//Arrêt de la VR
	engine.stopVR();

	//Destruction du moteur
	engine.destroy();
	return 0;
}