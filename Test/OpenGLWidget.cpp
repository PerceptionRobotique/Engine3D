#include "OpenGLWidget.h"

OpenGLWidget::OpenGLWidget(QWidget* parent, Qt::WindowFlags f)
	: QOpenGLWidget(parent, f)
    , cameraController(engine.getMainCamera(), this)
    , backgroundColor(0, 0, 0, 255)
    , quad{
          -1.0f, -1.0f, 0.0f,   0.0f, 0.0f
        ,  1.0f, -1.0f, 0.0f,   1.0f, 0.0f
        , -1.0f,  1.0f, 0.0f,   0.0f, 1.0f
        ,  1.0f,  1.0f, 0.0f,   1.0f, 1.0f
        ,  0.0f, -1.0f, 0.0f,   1.0f, 0.0f
        ,  0.0f,  1.0f, 0.0f,   1.0f, 1.0f
    }
    , quadIndices{
          0, 1, 2,
          1, 2, 3
    }
    , vboQuad(QOpenGLBuffer::VertexBuffer)
    , eboQuad(QOpenGLBuffer::IndexBuffer)
{
    setAttribute(Qt::WA_AcceptTouchEvents);

    cameraController.setTranslationSensitivity(7);
    cameraController.setRotationSensitivity(7);
    cameraController.setMouseCaptureEnabled(true);

    connect(&engine, SIGNAL(askUpdate()), this, SLOT(update()));
}

Engine3D& OpenGLWidget::getEngine()
{
    return engine;
}

void OpenGLWidget::initializeGL()
{
    engine.initialize();
    
    shader.create();
    shader.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/Viewer.vert");
    shader.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/Viewer.frag");
    shader.link();

    vboQuad.create();
    vboQuad.bind();
    vboQuad.allocate(quad.constData(), quad.count() * (int)sizeof(float));
    vboQuad.release();

    eboQuad.create();
    eboQuad.bind();
    eboQuad.allocate(quadIndices.constData(), quadIndices.count() * (int)sizeof(unsigned int));
    eboQuad.release();
}

void OpenGLWidget::paintGL()
{
    engine.update();

    context()->functions()->glViewport(0, 0, width() * screen()->devicePixelRatio(), height() * screen()->devicePixelRatio());
    context()->functions()->glClearColor(
        backgroundColor.redF(),
        backgroundColor.greenF(),
        backgroundColor.blueF(),
        backgroundColor.alphaF()
    );
    context()->functions()->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (shader.bind())
    {
        context()->functions()->glActiveTexture(GL_TEXTURE0);
        context()->functions()->glBindTexture(GL_TEXTURE_2D, engine.getMainCamera()->texture());
        vboQuad.bind();
        shader.enableAttributeArray("aPos");
        shader.setAttributeBuffer("aPos", GL_FLOAT, 0, 3, 5 * sizeof(float));
        shader.enableAttributeArray("aTexCoords");
        shader.setAttributeBuffer("aTexCoords", GL_FLOAT, 3 * sizeof(float), 2, 5 * sizeof(float));
        vboQuad.release();

        eboQuad.bind();
        context()->functions()->glDrawElements(GL_TRIANGLES, quadIndices.count(), GL_UNSIGNED_INT, 0);
        eboQuad.release();

        shader.disableAttributeArray("aPos");
        shader.disableAttributeArray("aTexCoords");
        context()->functions()->glBindTexture(GL_TEXTURE_2D, 0);
        shader.release();
    }
}

void OpenGLWidget::resizeGL(int w, int h)
{
    engine.getMainCamera()->setSize(w * screen()->devicePixelRatio(), h * screen()->devicePixelRatio());
}

void OpenGLWidget::mousePressEvent(QMouseEvent* e)
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

void OpenGLWidget::mouseMoveEvent(QMouseEvent* e)
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

void OpenGLWidget::mouseReleaseEvent(QMouseEvent* e)
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

void OpenGLWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
    Q_UNUSED(e);
}

void OpenGLWidget::wheelEvent(QWheelEvent* e)
{
    cameraController.mouseWheelMoved(e->angleDelta().x(), e->angleDelta().y());
}

bool OpenGLWidget::event(QEvent* e)
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
        return QOpenGLWidget::event(e);
}