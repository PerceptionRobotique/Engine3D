#include "Object3D.h"

Object3D::Object3D()
    : pose(1.0)
{

}

//TRANSFORMS
void Object3D::translate(const vec3 &_translation, const bool& _onGround)
{
    float pitch = getPitch();
    float roll = getRoll();
    if(_onGround)
    {
        setPitch(0);
        setRoll(0);
    }
    pose = glm::translate(pose, _translation);
    if(_onGround)
    {
        setPitch(pitch);
        setRoll(roll);
    }
}

void Object3D::rotate(float _angle, const vec3 &_axis, const bool &_verticalAxis)
{
    if(_axis.y && _verticalAxis) _angle *= (1 - abs(getPitch()) / 90.0f);

    pose = glm::rotate(pose, radians(_angle), _axis);

    if(_verticalAxis)
    {
        float roll = getRoll();
        if(abs((roll)) > 90.0f)
            roll = 180.0f;
        else
            roll = 0.0f;

        vec4 position = pose[3];
        setYawPitchRoll(getYaw(), getPitch(), roll);
        pose[3] = position;
    }
}

void Object3D::rotate(const mat4& _rotation, const bool &_verticalAxis)
{
    float pitch = getPitch();
    if(_verticalAxis) setPitch(0);
    vec3 rot;
    extractEulerAngleXYZ(_rotation, rot.x, rot.y, rot.z);
    pose = glm::rotate(pose, rot.x, vec3(1.0f, 0.0f, 0.0f));
    pose = glm::rotate(pose, rot.y, vec3(0.0f, 1.0f, 0.0f));
    pose = glm::rotate(pose, rot.z, vec3(0.0f, 0.0f, 1.0f));
    if(_verticalAxis) setPitch(pitch);
}

void Object3D::setYawPitchRoll(const float& _yaw, const float& _pitch, const float& _roll)
{
    vec4 position = pose[3];
    pose = glm::yawPitchRoll(radians(_yaw), radians(_pitch), radians(_roll));
    pose[3] = position;
}

void Object3D::setYaw(const float& _yaw)
{
    setYawPitchRoll(_yaw, getPitch(), getRoll());
}

void Object3D::setPitch(const float& _pitch)
{
    setYawPitchRoll(getYaw(), _pitch, getRoll());
}

void Object3D::setRoll(const float &_roll)
{
    setYawPitchRoll(getYaw(), getPitch(), _roll);
}

void Object3D::yawPitchRoll(const float& _yaw, const float& _pitch, const float& _roll)
{
    setYawPitchRoll(getYaw() + _yaw, getPitch() + _pitch, getRoll() + _roll);
}

void Object3D::yaw(const float& _yaw)
{
    setYawPitchRoll(getYaw() + _yaw, getPitch(), getRoll());
}

void Object3D::pitch(const float &_pitch)
{
    setYawPitchRoll(getYaw(), getPitch() + _pitch, getRoll());
}

void Object3D::roll(const float &_roll)
{
    setYawPitchRoll(getYaw(), getPitch(), getRoll() + _roll);
}

//SETTERS
void Object3D::setPose(const mat4 &_pose)
{
    pose = _pose;
}

void Object3D::setPosition(const vec3 &_position)
{
    pose[3] = vec4(_position, 1.0f);
}

void Object3D::setPositionX(const float &_tx)
{
    pose[3].x = _tx;
}

void Object3D::setPositionY(const float &_ty)
{
    pose[3].y = _ty;
}

void Object3D::setPositionZ(const float &_tz)
{
    pose[3].z = _tz;
}

void Object3D::setRotation(vec3 _rotation)
{
    _rotation = radians(_rotation);
    mat4 m(1.0);
    m = eulerAngleXYZ(_rotation.x, _rotation.y, _rotation.z);
    setRotation(m);
}

void Object3D::setRotation(mat4 _rotation)
{
    _rotation[3] = pose[3];
    pose = _rotation;
}

void Object3D::setRotationX(const float &_rx)
{
    setRotation(vec3(_rx, getRotation().y, getRotation().z));
}

void Object3D::setRotationY(const float &_ry)
{
    setRotation(vec3(getRotation().x, _ry, getRotation().z));
}

void Object3D::setRotationZ(const float &_rz)
{
    setRotation(vec3(getRotation().x, getRotation().y, _rz));
}

//GETTERS
mat4 Object3D::getPose() const
{
    return pose;
}

vec3 Object3D::getPosition() const
{
    return vec3(pose[3]);
}

vec3 Object3D::getRotation() const
{
    vec3 rot;
    extractEulerAngleXYZ(pose, rot.x, rot.y, rot.z);
    return degrees(rot);
}

mat4 Object3D::getRotationMatrix() const
{
    mat4 rot = pose;
    rot[3] = vec4(0, 0, 0, 1);
    return rot;
}

vec3 Object3D::getYawPitchRoll() const
{
    vec3 yawPitchRoll;
    extractEulerAngleYXZ(pose, yawPitchRoll.x, yawPitchRoll.y, yawPitchRoll.z);
    return degrees(yawPitchRoll);
}

float Object3D::getYaw() const
{
    return getYawPitchRoll().x;
}

float Object3D::getPitch() const
{
    return getYawPitchRoll().y;
}

float Object3D::getRoll() const
{
    return getYawPitchRoll().z;
}
