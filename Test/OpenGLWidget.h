#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTouchEvent>
#include <QColor>

#include <Engine3D.h>
#include <CameraController.h>

class OpenGLWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    OpenGLWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    Engine3D& getEngine();

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
    bool event(QEvent* e) override;

    QOpenGLShaderProgram shader;
    QVector<float> quad;
    QVector<unsigned int> quadIndices;
    QOpenGLBuffer vboQuad;
    QOpenGLBuffer eboQuad;
    Engine3D engine;
    CameraController cameraController;

    QColor backgroundColor;

private slots:
    void updateAsked();
};

#endif // OPENGLWIDGET_H
