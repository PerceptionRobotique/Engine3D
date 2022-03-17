#ifndef OBJECT3D_H
#define OBJECT3D_H

#include <glm/common.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "Engine3D_global.h"

using namespace glm;

class ENGINE3D_EXPORT Object3D
{
public:
    Object3D();

    //TRANSFORMS
    void translate(const vec3& _translation, const bool& _onGround = false);
    void rotate(float _angle, const vec3& _axis, const bool& _verticalAxis = false);
    void rotate(const mat4& _rotation, const bool& _verticalAxis = false);
    void yawPitchRoll(const float& _yaw, const float& _pitch, const float& _roll);
    void yaw(const float &_yaw);
    void pitch(const float& _pitch);
    void roll(const float& _roll);

    //SETTERS
    void setPose(const mat4& _pose);
    void setPosition(const vec3& _position);
    void setPositionX(const float& _tx);
    void setPositionY(const float& _ty);
    void setPositionZ(const float& _tz);
    void setRotation(vec3 _rotation);
    void setRotation(mat4 _rotation);
    void setRotationX(const float& _rx);
    void setRotationY(const float& _ry);
    void setRotationZ(const float& _rz);
    void setYawPitchRoll(const float& _yaw, const float& _pitch, const float& _roll);
    void setYaw(const float& _yaw);
    void setPitch(const float& _pitch);
    void setRoll(const float& _roll);

    //GETTERS
    mat4 getPose() const;
    vec3 getPosition() const;
    vec3 getRotation() const;
    mat4 getRotationMatrix() const;
    vec3 getYawPitchRoll() const;
    float getYaw() const;
    float getPitch() const;
    float getRoll() const;

private:
    mat4 pose;
};

#endif // OBJECT3D_H
