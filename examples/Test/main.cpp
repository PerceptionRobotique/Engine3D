#include <QGuiApplication>
#include <Engine3D.h>

using namespace MIS;

int main(int argc, char* argv[])
{
	QGuiApplication app(argc, argv);
	Q_UNUSED(app);

	Engine3D engine(Engine3D::DIRECT);
	engine.initialize();
	
	engine.destroy();
	return 0;
}