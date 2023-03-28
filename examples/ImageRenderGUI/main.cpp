#include <QApplication>
#include <QSettings>
#include <QFileDialog>
#include <Engine3D.h>

using namespace MIS;

int main(int argc, char* argv[])
{
	//Une interface est à afficher donc QApplication nécessaire (au lieu de QGuiApplication)
	QApplication app(argc, argv);

	//Création/Ouverture du fichier de paramètres
	QSettings settings("settings.ini", QSettings::IniFormat);

	//Création du moteur en mode de rendu direct
	Engine3D engine(Engine3D::DIRECT);
	//Initialisation du moteur
	engine.initialize();

	//Ouverture du modèle par l'utilisateur
	QString modelFileName = QFileDialog::getOpenFileName(nullptr, "Ouvrir un modèle 3D", settings.value("ModelFileName").toString(), "Modèle 3D (*.pts *.bin *.bini *.oct *.octi *.obj)");
	//Sélection de l'image de sauvegarde
	QString captureFileName = QFileDialog::getSaveFileName(nullptr, "Enregistrer capture", settings.value("CaptureFileName").toString(), "Fichier PNG (*.png)");
	if (!modelFileName.isEmpty() && !captureFileName.isEmpty())
	{
		settings.setValue("ModelFileName", modelFileName);
		settings.setValue("CaptureFileName", captureFileName);

		//Ouverture du modèle sélectionné
		engine.openModel(modelFileName);
		//Déplacement de la caméra de 5 mètres sur l'axe Z
		engine.getMainCamera()->translate(vec3(0, 0, 5));
		//Commande de LookAt pour que la caméra regarde vers le centre du modèle ouvert
		engine.getMainCamera()->lookAt(engine.getModel(0));
		//Capture et enregistrement d'une image dans un fichier
		engine.takePicture().save(captureFileName);
	}

	//Destruction du moteur
	engine.destroy();
	return 0;
}