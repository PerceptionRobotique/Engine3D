#include <QApplication>
#include <QSettings>
#include <QFileDialog>
#include <QInputDialog>
#include <QFileInfo>
#include <Engine3D.h>
#include <Model3DWriter.h>

#define DEFAULT_VERTEX_PER_NODE 10000

using namespace MIS;

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	Q_UNUSED(app);

	QSettings settings("settings.ini", QSettings::IniFormat);

	Engine3D engine(Engine3D::DIRECT);

	QString inFileName = QFileDialog::getOpenFileName(nullptr, "Ouvrir le fichier à convertir", settings.value("inFileName").toString(), "Nuage de points (*.pts *.bin *.bini *.oct *.octi)");
	QString selectedFilter = settings.value("selectedFilter").toString();
	QString outFileName = QFileDialog::getSaveFileName(nullptr, "Enregistrer le modèle sous...", settings.value("outFileName").toString(), "Modèle PTS (*.pts);;Modèle BIN (*.bin);;Modèle BINI (*.bini);;Modèle OCT (*.oct);;Modèle OCTI (*.octi)", &selectedFilter);
	if (!inFileName.isEmpty() && !outFileName.isEmpty())
	{
		settings.setValue("inFileName", inFileName);
		settings.setValue("outFileName", outFileName);
		settings.setValue("selectedFilter", selectedFilter);
		QFileInfo outFileInfo(outFileName);
		if (outFileInfo.suffix().isEmpty()) outFileName = outFileName.split(".")[0] + "." + selectedFilter.split("*")[1].remove(")");

		outFileInfo.setFile(outFileName);
		int vertexPerNode = DEFAULT_VERTEX_PER_NODE;
		if (outFileInfo.suffix().contains("oct"))
		{
			bool ok;
			vertexPerNode = QInputDialog::getInt(nullptr, "Nombre de vertex par noeud", "Vertex/Noeud", settings.value("vertexPerNode", DEFAULT_VERTEX_PER_NODE).toInt(), 1, 10000000, 1, &ok);
			if (!ok) vertexPerNode = DEFAULT_VERTEX_PER_NODE;
			else settings.setValue("vertexPerNode", vertexPerNode);
		}

		engine.openModel(inFileName);
		Model3DWriter mw;
		mw.write(engine.getModel(0), outFileName, vertexPerNode);
		mw.getWatcher().waitForFinished();
	}


	engine.destroy();
	return 0;
}