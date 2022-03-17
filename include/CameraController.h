#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include "Camera.h"

#include <QObject>
#include <QHash>
#include <QPoint>
#include <QTimer>
#include <QCursor>
#include <QGuiApplication>
#include <QWidget>
#include <QScreen>

#ifdef WITH_VR
#include "VRheadset.h"
#endif

#ifdef WITH_CONTROLLER
#include "Controller.h"
#endif

#define MOUSE_WHEEL_TIMEOUT 100

class ENGINE3D_EXPORT CameraController : public QObject
{
    Q_OBJECT
public:
#ifdef WITH_CONTROLLER
    enum Action {
        TRANSLATE_X,
        TRANSLATE_Y_PLUS,
        TRANSLATE_Y_MINUS,
        TRANSLATE_Z,
        ROTATE_X,
        ROTATE_Y,
        ROTATE_Z,
        YAW,
        PITCH,
        ROLL_PLUS,
        ROLL_MINUS
    };
#endif

    enum MouseButton{
        LEFT,
        MIDDLE,
        RIGHT
    };

    enum KeyButton{
        K_UP,
        K_DOWN,
        K_LEFT,
        K_RIGHT,
        K_SHIFT,
        K_SPACE,
        K_CTRL,
        K_Z,
        K_Q,
        K_S,
        K_D,
        K_A,
        K_E
    };

    CameraController(Camera* _camera, QWidget *_parent = nullptr);

    void setVerticalAxisEnabled(const bool& _verticalAxisEnabled);
    bool isVerticalAxisEnabled() const;
    void setOnGroundEnabled(const bool& _onGroundEnabled);
    bool isOnGroundEnabled() const;

    void setMouseCaptureEnabled(bool enabled);
    bool isMouseCaptureEnabled() const;
    void mousePressed(MouseButton mouseButton, int x, int y);
    void mouseMoved(MouseButton mouseButton, int x, int y);
    void mouseWheelMoved(int rx, int ry);
    void mouseReleased(MouseButton mouseButton);

    void touchBegin();
    void touchUpdate(const QVector<QPoint> &points);
    void touchEnd();

    void keyPressed(KeyButton key);
    void keyReleased(KeyButton key);

#ifdef WITH_VR
    void updateVRInputs(VRheadset* vrHeadset);
#endif

    float getTranslationSensitivity() const;
    void setTranslationSensitivity(float _translationSensitivity);
    float getRotationSensitivity() const;
    void setRotationSensitivity(float _rotationSensitivity);

    bool isMoving() const;

#ifdef WITH_CONTROLLER
    QHash<QString, QHash<Action, Controller::Input>>& getControllerProfiles();
    QString getCurrentControllerProfile() const;
    void setCurrentControllerProfile(QString newProfile);
#endif

signals:
    void movementFinished();

private slots:
    void mouseWheelFinished();
#ifdef WITH_CONTROLLER
    void updateController();
#endif

private:
    Camera* camera;
    QWidget* parent;

    bool verticalAxisEnabled;
    bool onGroundEnabled;

    QCursor cursor;
    bool mouseCaptureEnabled;
    int mouseMoveToSkip;
    QPoint savedCursorPosition;
    QHash<MouseButton, bool> mouseButtonPressed;
    QHash<MouseButton, QPoint> mousePreviousPos;
    QTimer mouseWheelTimer;

    QVector<QPoint> touchPoints;

    QHash<KeyButton, bool> keysDown;

#ifdef WITH_CONTROLLER
    QTimer controllerUpdater;
    QString currentControllerProfile;
    QHash<QString, QHash<Action, Controller::Input>> controllerProfiles;
#endif

    float translationSensitivity;
    float rotationSensitivity;
};

#endif // CAMERACONTROLLER_H
