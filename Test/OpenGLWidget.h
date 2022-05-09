#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTouchEvent>
#include <QColor>

#include <Engine3D.h>
#include <CameraController.h>

using namespace MIS;

class OpenGLWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    OpenGLWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~OpenGLWidget();

    Engine3D* getEngine();
    QImage grabImage();

private:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;
    bool event(QEvent* e) override;

    QOpenGLShaderProgram shader;
    QVector<float> quad;
    QVector<unsigned int> quadIndices;
    QOpenGLBuffer vboQuad;
    QOpenGLBuffer eboQuad;
    QOpenGLTexture texture;
    Engine3D engine;
    CameraController cameraController;

    QColor backgroundColor;
    QImage frame;

private slots:
    void updateEngine();
    void updateFrame(QImage frame);

signals:
    void askPaint();
};

#endif // OPENGLWIDGET_H
