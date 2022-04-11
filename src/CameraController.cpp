#include "CameraController.h"

CameraController::CameraController(Camera* _camera, QWidget *_parent)
    : QObject(_parent)
    , camera(_camera)
    , parent(_parent)
    , verticalAxisEnabled(true)
    , onGroundEnabled(false)
    , mouseCaptureEnabled(false)
    , mouseMoveToSkip(0)
    , keyboardKeysDown(0)
#ifdef WITH_CONTROLLER
    , controllerIsMoving(false)
    , currentControllerProfile("Standard")
#endif
    , translationSensitivity(1.0f)
    , rotationSensitivity(1.0f)
{
    connect(&mouseWheelTimer, SIGNAL(timeout()), this, SLOT(mouseWheelFinished()));

#ifdef WITH_CONTROLLER
    controllerProfiles["Standard"][TRANSLATE_X] = Controller::STICK_LEFT_X;
    controllerProfiles["Standard"][TRANSLATE_Y_PLUS] = Controller::TRIGGER_RIGHT;
    controllerProfiles["Standard"][TRANSLATE_Y_MINUS] = Controller::TRIGGER_LEFT;
    controllerProfiles["Standard"][TRANSLATE_Z] = Controller::STICK_LEFT_Y;
    controllerProfiles["Standard"][ROTATE_X] = Controller::STICK_RIGHT_Y;
    controllerProfiles["Standard"][ROTATE_Y] = Controller::STICK_RIGHT_X;
    controllerProfiles["Standard"][ROLL_PLUS] = Controller::R;
    controllerProfiles["Standard"][ROLL_MINUS] = Controller::L;

    controllerProfiles["Alternatif"][TRANSLATE_X] = Controller::STICK_RIGHT_X;
    controllerProfiles["Alternatif"][TRANSLATE_Y_PLUS] = Controller::DPAD_UP;
    controllerProfiles["Alternatif"][TRANSLATE_Y_MINUS] = Controller::DPAD_DOWN;
    controllerProfiles["Alternatif"][TRANSLATE_Z] = Controller::STICK_RIGHT_Y;
    controllerProfiles["Alternatif"][ROTATE_X] = Controller::STICK_LEFT_Y;
    controllerProfiles["Alternatif"][ROTATE_Y] = Controller::STICK_LEFT_X;

    connect(&controllerUpdater, SIGNAL(timeout()), this, SLOT(updateController()));
    controllerUpdater.start(10);
#endif
}

bool CameraController::isVerticalAxisEnabled() const
{
    return verticalAxisEnabled;
}

bool CameraController::isOnGroundEnabled() const
{
    return onGroundEnabled;
}

void CameraController::setMouseCaptureEnabled(bool enabled)
{
    mouseCaptureEnabled = enabled;
}

bool CameraController::isMouseCaptureEnabled() const
{
    return mouseCaptureEnabled;
}

void CameraController::mousePressed(MouseButton mouseButton, int x, int y)
{
    mouseButtonPressed[mouseButton] = true;
    if(mouseCaptureEnabled)
    {
        cursor.setShape(Qt::CursorShape::BlankCursor);
        QGuiApplication::setOverrideCursor(cursor);
        savedCursorPosition = QCursor::pos();
        parent->blockSignals(true);
        QCursor::setPos(parent->screen(), parent->parentWidget()->mapToGlobal(parent->geometry().center()));
        mouseMoveToSkip = 3;
        parent->blockSignals(false);
        mousePreviousPos[mouseButton] = parent->parentWidget()->mapToGlobal(parent->geometry().center()) - parent->parentWidget()->mapToGlobal(parent->geometry().topLeft());
    }
    else
    {
        mousePreviousPos[mouseButton].setX(x);
        mousePreviousPos[mouseButton].setY(y);
    }
    emit moving(true);
}

void CameraController::mouseMoved(MouseButton mouseButton, int x, int y)
{
    if(mouseMoveToSkip > 0)
    {
        parent->blockSignals(true);
        QCursor::setPos(parent->screen(), parent->parentWidget()->mapToGlobal(parent->geometry().center()));
        parent->blockSignals(false);
        mouseMoveToSkip--;
    }
    else
    {
        if(mouseButtonPressed[mouseButton])
        {
            QPointF r(x - mousePreviousPos[mouseButton].x()
                    ,y - mousePreviousPos[mouseButton].y());

            if(mouseCaptureEnabled)
            {
                parent->blockSignals(true);
                QCursor::setPos(parent->screen(), parent->parentWidget()->mapToGlobal(parent->geometry().center()));
                parent->blockSignals(false);
            }
            else
            {
                mousePreviousPos[mouseButton].setX(x);
                mousePreviousPos[mouseButton].setY(y);
            }

            switch(mouseButton)
            {
            case LEFT:
#ifdef ANDROID
                if(keysDown[K_CTRL])
                {
                    camera->translate(vec3(
                                          -translationSensitivity * r.x() / 50.0f,
                                           translationSensitivity * r.y() / 50.0f,
                                           0.0f),
                                      onGroundEnabled);
                }
                else
                {
#endif
                    camera->rotate((abs(camera->getRoll()) == 180.0f ? 1.0f : 1.0f) * rotationSensitivity * r.x() / 50.0f, vec3(0.0f, 1.0f, 0.0f), verticalAxisEnabled);
                    camera->rotate(rotationSensitivity * r.y() / 50.0f, vec3(1.0f, 0.0f, 0.0f), verticalAxisEnabled);
#ifdef ANDROID
                }
#endif
                break;

            case MIDDLE:
                break;

            case RIGHT:
                camera->translate(vec3(
                                      -translationSensitivity * r.x() / 50.0f,
                                       translationSensitivity * r.y() / 50.0f,
                                       0.0f),
                                  onGroundEnabled);
                break;
            }
        }
    }
}

void CameraController::mouseWheelMoved(int rx, int ry)
{
    camera->translate(vec3(-translationSensitivity * rx / 500.0f, 0.0f, -translationSensitivity * ry / 500.0f), onGroundEnabled);
    if(!mouseWheelTimer.isActive()) emit moving(true);
    mouseWheelTimer.start(MOUSE_WHEEL_TIMEOUT);
}

void CameraController::mouseReleased(MouseButton mouseButton)
{
    if(mouseCaptureEnabled)
    {
        QCursor::setPos(parent->screen(), savedCursorPosition);
        QGuiApplication::restoreOverrideCursor();
    }
    mouseButtonPressed[mouseButton] = false;
    if(!isMoving())
        emit moving(false);
}

void CameraController::touchBegin()
{
    emit moving(true);
}

void CameraController::touchUpdate(const QVector<QPoint>& points)
{
    if(touchPoints.count() != points.count())
    {
        touchPoints = points;
    }
    else
    {
        if(touchPoints.count() == 1)
        {
            int rx = points[0].x() - touchPoints[0].x();
            int ry = points[0].y() - touchPoints[0].y();

            camera->rotate((abs(camera->getRoll()) == 180.0f ? -1.0f : 1.0f) * rotationSensitivity * rx / 50.0f, vec3(0.0f, 1.0f, 0.0f), verticalAxisEnabled);
            camera->rotate(rotationSensitivity * ry / 50.0f, vec3(1.0f, 0.0f, 0.0f), verticalAxisEnabled);

            touchPoints = points;
        }
        else if(touchPoints.count() == 2)
        {
            float rx = (points[0].x() - touchPoints[0].x() + points[1].x() - touchPoints[1].x()) / 2;
            float ry = (points[0].y() - touchPoints[0].y() + points[1].y() - touchPoints[1].y()) / 2;
            float rz = glm::distance(vec2(points[0].x(), points[0].y()), vec2(points[1].x(), points[1].y()))
                     - glm::distance(vec2(touchPoints[0].x(), touchPoints[0].y()), vec2(touchPoints[1].x(), touchPoints[1].y()));

            camera->translate(vec3(
                                  -translationSensitivity * rx / 50.0f,
                                   translationSensitivity * ry / 50.0f,
                                  -translationSensitivity * rz / 50.0f),
                              onGroundEnabled);

            QLineF line1(touchPoints[0], touchPoints[1]);
            QLineF line2(points[0], points[1]);

            float az = line1.angle() - line2.angle();
            camera->rotate(az, vec3(0.0f, 0.0f, 1.0f), verticalAxisEnabled);

            touchPoints = points;
        }
    }
}

void CameraController::touchEnd()
{
    touchPoints.clear();
    if(!isMoving()) emit moving(false);
}

void CameraController::keyPressed(CameraController::KeyButton key)
{
    switch(key)
    {
    case K_UP:
        keysDown[K_UP] = true;
        camera->translate(vec3(0, 0, -translationSensitivity), onGroundEnabled);
        break;

    case K_DOWN:
        keysDown[K_DOWN] = true;
        camera->translate(vec3(0, 0, translationSensitivity), onGroundEnabled);
        break;

    case K_LEFT:
        keysDown[K_LEFT] = true;
        camera->translate(vec3(-translationSensitivity, 0, 0), onGroundEnabled);
        break;

    case K_RIGHT:
        keysDown[K_RIGHT] = true;
        camera->translate(vec3(translationSensitivity, 0, 0), onGroundEnabled);
        break;

    case K_Z:
        keysDown[K_Z] = true;
        camera->rotate(rotationSensitivity, vec3(1, 0, 0), verticalAxisEnabled);
        break;

    case K_Q:
        keysDown[K_Q] = true;
        camera->rotate(rotationSensitivity, vec3(0, 1, 0), verticalAxisEnabled);
        break;

    case K_S:
        keysDown[K_S] = true;
        camera->rotate(-rotationSensitivity, vec3(1, 0, 0), verticalAxisEnabled);
        break;

    case K_D:
        keysDown[K_D] = true;
        camera->rotate(-rotationSensitivity, vec3(0, 1, 0), verticalAxisEnabled);
        break;

    case K_A:
        keysDown[K_A] = true;
        camera->translate(vec3(0, translationSensitivity, 0), onGroundEnabled);
        break;

    case K_E:
        keysDown[K_E] = true;
        camera->translate(vec3(0, -translationSensitivity, 0), onGroundEnabled);
        break;

    default:
        break;
    }

    keyboardKeysDown = 0;
    for (KeyButton button : keysDown.keys())
        if (keysDown[button]) keyboardKeysDown++;

    if (isMoving()) emit moving(true);
}

void CameraController::keyReleased(KeyButton key)
{
    switch(key)
    {
    case K_UP:
        keysDown[K_UP] = false;
        break;

    case K_DOWN:
        keysDown[K_DOWN] = false;
        break;

    case K_LEFT:
        keysDown[K_LEFT] = false;
        break;

    case K_RIGHT:
        keysDown[K_RIGHT] = false;
        break;

    case K_Z:
        keysDown[K_Z] = false;
        break;

    case K_Q:
        keysDown[K_Q] = false;
        break;

    case K_S:
        keysDown[K_S] = false;
        break;

    case K_D:
        keysDown[K_D] = false;
        break;

    case K_A:
        keysDown[K_A] = false;
        break;

    case K_E:
        keysDown[K_E] = false;
        break;

    default:
        break;
    }

    keyboardKeysDown = 0;
    for (KeyButton button : keysDown.keys())
        if (keysDown[button]) keyboardKeysDown++;

    if (!isMoving()) emit moving(false);
}

#ifdef WITH_VR
void CameraController::updateVRInputs(VRheadset* vrHeadset)
{
    for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
    {
        vr::VRControllerState_t state;
        if(vrHeadset->m_pHMD->GetControllerState(unDevice, &state, sizeof(state)))
        {
            if (vrHeadset->m_pHMD->GetInt32TrackedDeviceProperty(unDevice, vr::Prop_ControllerRoleHint_Int32) == vr::TrackedControllerRole_LeftHand)
            {
                camera->translate(vec3(translationSensitivity * state.rAxis->x / 50.0f, 0.0f, -translationSensitivity * state.rAxis->y / 50.0f), true);
            }
            else if(vrHeadset->m_pHMD->GetInt32TrackedDeviceProperty(unDevice, vr::Prop_ControllerRoleHint_Int32) == vr::TrackedControllerRole_RightHand)
            {
                camera->translate(vec3(0, translationSensitivity * state.rAxis->y / 50.0f, 0), true);
                vrHeadset->m_mat4EyeRotOffset = glm::rotate(vrHeadset->m_mat4EyeRotOffset, -rotationSensitivity * state.rAxis[0].x / 100.0f, glm::vec3(0, 1, 0));
            }
        }
    }
}
#endif

float CameraController::getTranslationSensitivity() const
{
    return translationSensitivity;
}

float CameraController::getRotationSensitivity() const
{
    return rotationSensitivity;
}

bool CameraController::isMoving() const
{
    bool moving = false;

    foreach (MouseButton button, mouseButtonPressed.keys())
        moving |= mouseButtonPressed[button];

    moving |= mouseWheelTimer.isActive();
    moving |= touchPoints.count() > 0;
    moving |= keyboardKeysDown > 0;
#ifdef WITH_CONTROLLER
    moving |= controllerIsMoving;
#endif

    return moving;
}

#ifdef WITH_CONTROLLER
QHash<QString, QHash<CameraController::Action, Controller::Input>>& CameraController::getControllerProfiles()
{
    return controllerProfiles;
}

QString CameraController::getCurrentControllerProfile() const
{
    return currentControllerProfile;
}
#endif

void CameraController::setTranslationSensitivity(int _translationSensitivity)
{
    translationSensitivity = _translationSensitivity;
}

void CameraController::setRotationSensitivity(int _rotationSensitivity)
{
    rotationSensitivity = _rotationSensitivity;
}

void CameraController::setVerticalAxisEnabled(const bool& _verticalAxisEnabled)
{
    verticalAxisEnabled = _verticalAxisEnabled;
}

void CameraController::setOnGroundEnabled(const bool& _onGroundEnabled)
{
    onGroundEnabled = _onGroundEnabled;
}

#ifdef WITH_CONTROLLER
void CameraController::setCurrentControllerProfile(QString newProfile)
{
    currentControllerProfile = newProfile;
}
#endif

void CameraController::mouseWheelFinished()
{
    mouseWheelTimer.stop();
    emit moving(isMoving());
}

#ifdef WITH_CONTROLLER
void CameraController::updateController()
{
    Controller::update();
    bool controllerMoving = false;

    for (Action action : controllerProfiles[currentControllerProfile].keys())
    {
        if (Controller::getInput(controllerProfiles[currentControllerProfile][action]))
        {
            controllerMoving = true;
            switch (action)
            {
            case TRANSLATE_X:
                camera->translate(vec3(translationSensitivity * Controller::getInput(controllerProfiles[currentControllerProfile][TRANSLATE_X]) / 10.0f, 0.0f, 0.0f), onGroundEnabled);
                break;

            case TRANSLATE_Y_PLUS:
                camera->translate(vec3(0.0f, translationSensitivity * Controller::getInput(controllerProfiles[currentControllerProfile][TRANSLATE_Y_PLUS]) / 10.0f, 0.0f), onGroundEnabled);
                break;

            case TRANSLATE_Y_MINUS:
                camera->translate(vec3(0.0f, -translationSensitivity * Controller::getInput(controllerProfiles[currentControllerProfile][TRANSLATE_Y_MINUS]) / 10.0f, 0.0f), onGroundEnabled);
                break;

            case TRANSLATE_Z:
                camera->translate(vec3(0.0f, 0.0f, -translationSensitivity * Controller::getInput(controllerProfiles[currentControllerProfile][TRANSLATE_Z]) / 10.0f), onGroundEnabled);
                break;

            case ROTATE_X:
                camera->rotate(rotationSensitivity * Controller::getInput(controllerProfiles[currentControllerProfile][ROTATE_X]) / 5.0f, vec3(1, 0, 0), verticalAxisEnabled);
                break;

            case ROTATE_Y:
                camera->rotate(-rotationSensitivity * Controller::getInput(controllerProfiles[currentControllerProfile][ROTATE_Y]) / 5.0f, vec3(0, 1, 0), verticalAxisEnabled);
                break;

            case ROLL_PLUS:
                camera->roll(rotationSensitivity * Controller::getInput(controllerProfiles[currentControllerProfile][ROLL_PLUS]) / 3.0f);
                break;

            case ROLL_MINUS:
                camera->roll(-rotationSensitivity * Controller::getInput(controllerProfiles[currentControllerProfile][ROLL_MINUS]) / 3.0f);
                break;
            }
        }
    }

    if (controllerMoving != controllerIsMoving)
    {
        controllerIsMoving = controllerMoving;
        emit moving(isMoving());
    }
}
#endif
