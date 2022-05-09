#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QTouchEvent>

#include <Engine3D.h>
#include <CameraController.h>

using namespace MIS;

class GraphicsView : public QGraphicsView
{
	Q_OBJECT
public:
	GraphicsView(QWidget* parent = nullptr);
	~GraphicsView() override;

	Engine3D* getEngine();
	QImage grabImage();

private:
	QGraphicsScene scene;
	QGraphicsPixmapItem pixmapItem;

	Engine3D engine;
	CameraController cameraController;

	void mousePressEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
	void mouseDoubleClickEvent(QMouseEvent* e) override;
	void wheelEvent(QWheelEvent* e) override;
	void keyPressEvent(QKeyEvent* e) override;
	void keyReleaseEvent(QKeyEvent* e) override;
	void resizeEvent(QResizeEvent* e) override;
	bool event(QEvent* e) override;

private slots:
	void updateEngine();
	void updateFrame(QImage frame);
};

#endif // GRAPHICSVIEW_H