#include <QGuiApplication>
#include <Engine3D.h>

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
	//Initialisation du moteur
	engine.initialize();
	engine.getMainCamera()->setBackgroundColor(Qt::white);

	//Ouverture du modèle
	engine.openModel("../Model/Suzanne.obj");
	//Déplacement de la caméra de 3 mètres sur l'axe Z (mouvement de recul)
	engine.getMainCamera()->translate(vec3(0, 0, 3));
	//Capture d'une image et sauvegarde dans un fichier
	engine.takePicture().save("capture.png");

	//Destruction du moteur
	engine.destroy();
	return 0;
}