#include "Object3DQt.h"

Object3DQt::Object3DQt(QObject *parent)
    : QObject(parent)
{

}

//TRANSFORMS
void Object3DQt::translate(const vec3 &_translation, const bool &_onGround)
{
    Object3D::translate(_translation, _onGround);
    emit objectChanged();
}

void Object3DQt::rotate(const float &_angle, const vec3 &_axis, const bool &_verticalAxis)
{
    Object3D::rotate(_angle, _axis, _verticalAxis);
    emit objectChanged();
}

void Object3DQt::rotate(const mat4 &_rotation, const bool &_verticalAxis)
{
    Object3D::rotate(_rotation, _verticalAxis);
    emit objectChanged();
}

void Object3DQt::setYawPitchRoll(const float& _yaw, const float& _pitch, const float& _roll)
{
    Object3D::setYawPitchRoll(_yaw, _pitch, _roll);
    emit objectChanged();
}

void Object3DQt::setYaw(const float &_yaw)
{
    Object3D::setYaw(_yaw);
    emit objectChanged();
}

void Object3DQt::setPitch(const float &_pitch)
{
    Object3D::setPitch(_pitch);
    emit objectChanged();
}

void Object3DQt::setRoll(const float &_roll)
{
    Object3D::setRoll(_roll);
    emit objectChanged();
}

void Object3DQt::yaw(const float& _yaw)
{
    Object3D::yaw(_yaw);
    emit objectChanged();
}

void Object3DQt::pitch(const float &_pitch)
{
    Object3D::pitch(_pitch);
    emit objectChanged();
}

void Object3DQt::roll(const float &_roll)
{
    Object3D::roll(_roll);
    emit objectChanged();
}

//SETTERS
void Object3DQt::setPose(const mat4 &_pose)
{
    Object3D::setPose(_pose);
    emit objectChanged();
}

void Object3DQt::setPosition(const vec3 &_position)
{
    Object3D::setPosition(_position);
    emit objectChanged();
}

void Object3DQt::setPositionX(const float &_tx)
{
    Object3D::setPositionX(_tx);
    emit objectChanged();
}

void Object3DQt::setPositionY(const float &_ty)
{
    Object3D::setPositionY(_ty);
    emit objectChanged();
}

void Object3DQt::setPositionZ(const float &_tz)
{
    Object3D::setPositionZ(_tz);
    emit objectChanged();
}

void Object3DQt::setPositionX(const double& _tx)
{
    setPositionX((float)_tx);
}

void Object3DQt::setPositionY(const double& _ty)
{
    setPositionY((float)_ty);
}

void Object3DQt::setPositionZ(const double& _tz)
{
    setPositionZ((float)_tz);
}

void Object3DQt::setRotation(vec3 _rotation)
{
    Object3D::setRotation(_rotation);
    emit objectChanged();
}

void Object3DQt::setRotation(mat4 _rotation)
{
    Object3D::setRotation(_rotation);
    emit objectChanged();
}

void Object3DQt::setRotationX(const float &_rx)
{
    Object3D::setRotationX(_rx);
    emit objectChanged();
}

void Object3DQt::setRotationY(const float &_ry)
{
    Object3D::setRotationY(_ry);
    emit objectChanged();
}

void Object3DQt::setRotationZ(const float &_rz)
{
    Object3D::setRotationZ(_rz);
    emit objectChanged();
}

void Object3DQt::setRotationX(const double& _rx)
{
    setRotationX((float)_rx);
}

void Object3DQt::setRotationY(const double& _ry)
{
    setRotationY((float)_ry);
}

void Object3DQt::setRotationZ(const double& _rz)
{
    setRotationZ((float)_rz);
}