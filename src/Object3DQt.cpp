#include "Object3DQt.h"

namespace MIS
{

    Object3DQt::Object3DQt(QObject* parent)
        : QObject(parent)
    {

    }

    //TRANSFORMS
    void Object3DQt::translate(const vec3& _translation, const bool& _onGround)
    {
        Object3D::translate(_translation, _onGround);
        emit objectMoved();
    }

    void Object3DQt::translate(const float& tx, const float& ty, const float& tz, const bool& onGround)
    {
        Object3D::translate(tx, ty, tz, onGround);
        emit objectMoved();
    }

    void Object3DQt::rotate(const float& _angle, const vec3& _axis, const bool& _verticalAxis)
    {
        Object3D::rotate(_angle, _axis, _verticalAxis);
        emit objectMoved();
    }

    void Object3DQt::rotate(const mat4& _rotation, const bool& _verticalAxis)
    {
        Object3D::rotate(_rotation, _verticalAxis);
        emit objectMoved();
    }

    void Object3DQt::rotateXYZ(const vec3& rotation, const bool& verticalAxis)
    {
        Object3D::rotateXYZ(rotation, verticalAxis);
        emit objectMoved();
    }

    void Object3DQt::rotateYXZ(const vec3& rotation, const bool& verticalAxis)
    {
        Object3D::rotateYXZ(rotation, verticalAxis);
        emit objectMoved();
    }

    void Object3DQt::rotateZYX(const vec3& rotation, const bool& verticalAxis)
    {
        Object3D::rotateZYX(rotation, verticalAxis);
        emit objectMoved();
    }

    void Object3DQt::setYawPitchRoll(const float& _yaw, const float& _pitch, const float& _roll)
    {
        Object3D::setYawPitchRoll(_yaw, _pitch, _roll);
        emit objectMoved();
    }

    void Object3DQt::setYaw(const float& _yaw)
    {
        Object3D::setYaw(_yaw);
        emit objectMoved();
    }

    void Object3DQt::setPitch(const float& _pitch)
    {
        Object3D::setPitch(_pitch);
        emit objectMoved();
    }

    void Object3DQt::setRoll(const float& _roll)
    {
        Object3D::setRoll(_roll);
        emit objectMoved();
    }

    void Object3DQt::setYaw(const double& _yaw)
    {
        setYaw((float)_yaw);
    }

    void Object3DQt::setPitch(const double& _pitch)
    {
        setPitch((float)_pitch);
    }

    void Object3DQt::setRoll(const double& _roll)
    {
        setRoll((float)_roll);
    }

    void Object3DQt::yaw(const float& _yaw)
    {
        Object3D::yaw(_yaw);
        emit objectMoved();
    }

    void Object3DQt::pitch(const float& _pitch)
    {
        Object3D::pitch(_pitch);
        emit objectMoved();
    }

    void Object3DQt::roll(const float& _roll)
    {
        Object3D::roll(_roll);
        emit objectMoved();
    }

    //SETTERS
    void Object3DQt::setPose(const mat4& _pose)
    {
        Object3D::setPose(_pose);
        emit objectMoved();
    }

    void Object3DQt::setPosition(const vec3& _position)
    {
        Object3D::setPosition(_position);
        emit objectMoved();
    }

    void Object3DQt::setPosition(const float& tx, const float& ty, const float& tz)
    {
        Object3D::setPosition(tx, ty, tz);
        emit objectMoved();
    }

    void Object3DQt::setPositionX(const float& _tx)
    {
        Object3D::setPositionX(_tx);
        emit objectMoved();
    }

    void Object3DQt::setPositionY(const float& _ty)
    {
        Object3D::setPositionY(_ty);
        emit objectMoved();
    }

    void Object3DQt::setPositionZ(const float& _tz)
    {
        Object3D::setPositionZ(_tz);
        emit objectMoved();
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
        emit objectMoved();
    }

    void Object3DQt::setRotation(mat4 _rotation)
    {
        Object3D::setRotation(_rotation);
        emit objectMoved();
    }

    void Object3DQt::setRotationX(const float& _rx)
    {
        Object3D::setRotationX(_rx);
        emit objectMoved();
    }

    void Object3DQt::setRotationY(const float& _ry)
    {
        Object3D::setRotationY(_ry);
        emit objectMoved();
    }

    void Object3DQt::setRotationZ(const float& _rz)
    {
        Object3D::setRotationZ(_rz);
        emit objectMoved();
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

    mat4 stringToMat4(QString matString)
    {
        matString = matString.remove("mat4x4").remove("(").remove(")").replace(", ", " ").replace(",", ".");
        QStringList values = matString.split(" ");
        mat4 pose(1.0);
        if (!matString.isEmpty())
        {
            for (unsigned int i = 0; i < 4; i++)
            {
                for (unsigned int j = 0; j < 4; j++)
                {
                    pose[i][j] = values[i * 4 + j].toFloat();
                }
            }
        }
        return pose;
    }

    vec3 stringToVec3(QString vecString)
    {
        vecString = vecString.remove("vec3").remove("(").remove(")").remove(",");
        QStringList values = vecString.split(" ");
        vec3 vec;
        if (!vecString.isEmpty())
        {
            for (unsigned int i = 0; i < 3; i++)
            {
                vec[i] = values[i].toFloat();
            }
        }
        return vec;
    }

}