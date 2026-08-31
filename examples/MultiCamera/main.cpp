#include <QGuiApplication>
#include <Engine3D.h>
#include <Camera.h>

//namespace de Engine3D
using namespace MIS;

int main(int argc, char* argv[])
{
	//Pas d'interface à afficher donc QGuiApplication suffisant
	QGuiApplication app(argc, argv);
	//app est inutilisé dans ce programme (mais bien nécessaire)
	Q_UNUSED(app);

	//Création du moteur en mode de rendu direct
	Engine3D engine(Engine3D::DIRECT);

	//Caméra principale (récupérée par takePicture() / getFrame())
	Camera* mainCamera = engine.getMainCamera();
	mainCamera->setBackgroundColor(Qt::white);

	//Caméra secondaire : placée à un autre endroit de la scène
	Camera* sideCamera = new Camera();
	sideCamera->setBackgroundColor(Qt::lightGray);
	sideCamera->translate(vec3(2, 0.5, 3));
	sideCamera->lookAt(vec3(0, 0, 0));
	engine.addCamera(sideCamera);

	//Initialisation du moteur (toutes les caméras ajoutées sont rendues)
	engine.initialize();

	//Ouverture du modèle
	engine.openModel("../Model/Suzanne.obj");
	//Déplacement de la caméra principale de 3 mètres sur l'axe Z (mouvement de recul)
	mainCamera->translate(vec3(0, 0, 3));

	//Rendu de la scène : takePicture() renvoie l'image de la caméra principale
	engine.takePicture().save("capture_main.png");

	//Le flux des autres caméras est récupérable via getFrame() sans contexte
	//OpenGL courant : l'image a été capturée pendant le rendu ci-dessus.
	sideCamera->getFrame().save("capture_side.png");

	//Destruction du moteur (libère aussi les caméras ajoutées)
	engine.destroy();
	return 0;
}
