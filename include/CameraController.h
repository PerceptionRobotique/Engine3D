#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <QObject>
#include <QPoint>
#include <QTimer>
#include <QCursor>
#include <QGuiApplication>
#include <QWidget>
#include <QScreen>

#include "Camera.h"
#include "Engine3D.h"

#ifdef HAVE_VR
#include "VRheadset.h"
#endif

#ifdef HAVE_CONTROLLER
#include "Controller.h"
#endif

#define MOUSE_WHEEL_TIMEOUT 100

namespace MIS
{

    class ENGINE3D_EXPORT CameraController : public QObject
    {
        Q_OBJECT
    public:
#ifdef HAVE_CONTROLLER
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

        enum MouseButton {
            LEFT,
            MIDDLE,
            RIGHT
        };

        enum KeyButton {
            K_UP,
            K_DOWN,
            K_LEFT,
            K_RIGHT,
            K_Z,
            K_Q,
            K_S,
            K_D,
            K_A,
            K_E
        };

        CameraController(Camera* _camera, QWidget* _parent = nullptr);
        CameraController(Engine3D* _engine, QWidget* _parent = nullptr);

        bool isActive() const;
        bool isVerticalAxisEnabled() const;
        bool isOnGroundEnabled() const;

        bool isMouseCaptureEnabled() const;
        void mousePressed(MouseButton mouseButton, int x, int y);
        void mouseMoved(MouseButton mouseButton, int x, int y);
        void mouseWheelMoved(int rx, int ry);
        void mouseReleased(MouseButton mouseButton);

        void touchBegin();
        void touchUpdate(const QVector<QPoint>& points);
        void touchEnd();

        void keyPressed(KeyButton key);
        void keyReleased(KeyButton key);

        float getTranslationSensitivity() const;
        float getRotationSensitivity() const;

        bool isMoving() const;

#ifdef HAVE_CONTROLLER
        QHash<QString, QHash<Action, Controller::Input>>& getControllerProfiles();
        QString getCurrentControllerProfile() const;
#endif

    signals:
        void moving(bool moving);

    public slots:
        void setActive(bool _active);
        void setTranslationSensitivity(int _translationSensitivity);
        void setRotationSensitivity(int _rotationSensitivity);
        void setVerticalAxisEnabled(const bool& _verticalAxisEnabled);
        void setOnGroundEnabled(const bool& _onGroundEnabled);
        void setMouseCaptureEnabled(bool enabled);
#ifdef HAVE_CONTROLLER
        void setCurrentControllerProfile(QString newProfile);
#endif

#ifdef HAVE_VR
        void setVRheadset(VRheadset* vrHeadset);
        void setVRInputsUpdaterEnabled(bool enabled);
        void updateVRInputs(VRheadset* vrHeadset = nullptr);
#endif

    private slots:
        void mouseWheelFinished();
#ifdef HAVE_CONTROLLER
        void updateController();
#endif

    private:
        Camera* camera;
        QWidget* parent;

        bool active;
        bool verticalAxisEnabled;
        bool onGroundEnabled;

        float translationSensitivity;
        float rotationSensitivity;

        QCursor cursor;
        bool mouseCaptureEnabled;
        int mouseMoveToSkip;
        QPoint savedCursorPosition;
        QHash<MouseButton, bool> mouseButtonPressed;
        QHash<MouseButton, QPoint> mousePreviousPos;
        QTimer mouseWheelTimer;

        QVector<QPoint> touchPoints;

        QHash<KeyButton, bool> keysDown;
        int keyboardKeysDown;

#ifdef HAVE_CONTROLLER
        bool controllerIsMoving;
        QTimer controllerUpdater;
        QString currentControllerProfile;
        QHash<QString, QHash<Action, Controller::Input>> controllerProfiles;
#endif

#ifdef HAVE_VR
        QTimer vrInputsUpdater;
        VRheadset* vr;
#endif
    };

}

#endif // CAMERACONTROLLER_H
