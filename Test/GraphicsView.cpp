#include "GraphicsView.h"

GraphicsView::GraphicsView(QWidget* parent)
	: QGraphicsView(parent)
    , engine(Engine3D::THREADED)
    , cameraController(&engine, this)
{
    setAttribute(Qt::WA_AcceptTouchEvents);

    connect(&engine, SIGNAL(askUpdate()), this, SLOT(updateEngine()));
    connect(&engine, SIGNAL(frameReady(QImage)), this, SLOT(updateFrame(QImage)));

    setScene(&scene);
    setBackgroundBrush(QBrush(Qt::black, Qt::SolidPattern));
    scene.addItem(&pixmapItem);

    engine.initialize();
    engine.setFrameCounterEnabled(true);
    engine.getMainCamera()->setSamples(8);

    cameraController.setTranslationSensitivity(5);
    cameraController.setRotationSensitivity(5);
}

GraphicsView::~GraphicsView()
{
	engine.destroy();
}

Engine3D* GraphicsView::getEngine()
{
    return &engine;
}

void GraphicsView::resizeEvent(QResizeEvent* e)
{
    QGraphicsView::resizeEvent(e);
    updateFrame(engine.getFrame());
}

void GraphicsView::mousePressEvent(QMouseEvent* e)
{
    if (e->source() == Qt::MouseEventNotSynthesized)
    {
        if (e->button() == Qt::LeftButton)
        {
            cameraController.mousePressed(CameraController::LEFT, e->pos().x(), e->pos().y());
            e->accept();
        }
        else if (e->button() == Qt::RightButton)
        {
            cameraController.mousePressed(CameraController::RIGHT, e->pos().x(), e->pos().y());
            e->accept();
        }
        else
            e->ignore();
    }
    else
        e->ignore();
}

void GraphicsView::mouseMoveEvent(QMouseEvent* e)
{
    if (!signalsBlocked())
    {
        if (e->source() == Qt::MouseEventNotSynthesized)
        {
            if (e->buttons().testFlag(Qt::LeftButton))
            {
                cameraController.mouseMoved(CameraController::LEFT, e->pos().x(), e->pos().y());
                e->accept();
            }
            if (e->buttons().testFlag(Qt::RightButton))
            {
                cameraController.mouseMoved(CameraController::RIGHT, e->pos().x(), e->pos().y());
                e->accept();
            }
            if (!e->isAccepted())
                e->ignore();
        }
        else
            e->ignore();
    }
}

void GraphicsView::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->source() == Qt::MouseEventNotSynthesized)
    {
        if (e->button() == Qt::LeftButton)
        {
            cameraController.mouseReleased(CameraController::LEFT);
            e->accept();
        }
        else if (e->button() == Qt::RightButton)
        {
            cameraController.mouseReleased(CameraController::RIGHT);
            e->accept();
        }
        else
            e->ignore();
    }
    else
        e->ignore();
}

void GraphicsView::mouseDoubleClickEvent(QMouseEvent* e)
{
    Q_UNUSED(e);
}

void GraphicsView::keyPressEvent(QKeyEvent* e)
{
    switch (e->key())
    {
    case Qt::Key_Up:
        cameraController.keyPressed(CameraController::K_UP);
        e->accept();
        break;

    case Qt::Key_Down:
        cameraController.keyPressed(CameraController::K_DOWN);
        e->accept();
        break;

    case Qt::Key_Left:
        cameraController.keyPressed(CameraController::K_LEFT);
        e->accept();
        break;

    case Qt::Key_Right:
        cameraController.keyPressed(CameraController::K_RIGHT);
        e->accept();
        break;

    case Qt::Key_Z:
        cameraController.keyPressed(CameraController::K_Z);
        e->accept();
        break;

    case Qt::Key_Q:
        cameraController.keyPressed(CameraController::K_Q);
        e->accept();
        break;

    case Qt::Key_S:
        cameraController.keyPressed(CameraController::K_S);
        e->accept();
        break;

    case Qt::Key_D:
        cameraController.keyPressed(CameraController::K_D);
        e->accept();
        break;

    case Qt::Key_A:
        cameraController.keyPressed(CameraController::K_A);
        e->accept();
        break;

    case Qt::Key_E:
        cameraController.keyPressed(CameraController::K_E);
        e->accept();
        break;

    default:
        e->ignore();
        break;
    }
}

void GraphicsView::keyReleaseEvent(QKeyEvent* e)
{
    switch (e->key())
    {
    case Qt::Key_Up:
        cameraController.keyReleased(CameraController::K_UP);
        e->accept();
        break;

    case Qt::Key_Down:
        cameraController.keyReleased(CameraController::K_DOWN);
        e->accept();
        break;

    case Qt::Key_Left:
        cameraController.keyReleased(CameraController::K_LEFT);
        e->accept();
        break;

    case Qt::Key_Right:
        cameraController.keyReleased(CameraController::K_RIGHT);
        e->accept();
        break;

    case Qt::Key_Z:
        cameraController.keyReleased(CameraController::K_Z);
        e->accept();
        break;

    case Qt::Key_Q:
        cameraController.keyReleased(CameraController::K_Q);
        e->accept();
        break;

    case Qt::Key_S:
        cameraController.keyReleased(CameraController::K_S);
        e->accept();
        break;

    case Qt::Key_D:
        cameraController.keyReleased(CameraController::K_D);
        e->accept();
        break;

    case Qt::Key_A:
        cameraController.keyReleased(CameraController::K_A);
        e->accept();
        break;

    case Qt::Key_E:
        cameraController.keyReleased(CameraController::K_E);
        e->accept();
        break;

    default:
        e->ignore();
        break;
    }
}

void GraphicsView::wheelEvent(QWheelEvent* e)
{
    cameraController.mouseWheelMoved(e->angleDelta().x(), e->angleDelta().y());
}

bool GraphicsView::event(QEvent* e)
{
    if (e->type() == QEvent::TouchBegin)
    {
        QTouchEvent* te = dynamic_cast<QTouchEvent*>(e);
        QVector<QPoint> points;
        for (unsigned int i = 0; i < te->points().count(); i++)
        {
            points.append(QPoint(te->points()[i].position().x(), te->points()[i].position().y()));
        }

        cameraController.touchBegin();
        cameraController.touchUpdate(points);

        e->accept();
    }
    else if (e->type() == QEvent::TouchUpdate)
    {
        QTouchEvent* te = dynamic_cast<QTouchEvent*>(e);
        QVector<QPoint> points;
        for (unsigned int i = 0; i < te->points().count(); i++)
        {
            points.append(QPoint(te->points()[i].position().x(), te->points()[i].position().y()));
        }

        cameraController.touchUpdate(points);

        e->accept();
    }
    else if (e->type() == QEvent::TouchEnd)
    {
        cameraController.touchEnd();

        e->accept();
    }
    else if (e->type() == QEvent::TouchCancel)
    {
        cameraController.touchEnd();

        e->accept();
    }
    else
        e->ignore();

    if (e->isAccepted())
        return true;
    else
        return QGraphicsView::event(e);
}

void GraphicsView::updateEngine()
{
    engine.update();
}

void GraphicsView::updateFrame(QImage frame)
{
    pixmapItem.setPixmap(QPixmap::fromImage(frame));
    fitInView(&pixmapItem, Qt::KeepAspectRatio);
}