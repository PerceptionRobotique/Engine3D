#ifndef OBJECT3DQT_H
#define OBJECT3DQT_H

#include <QObject>

#include "Object3D.h"

namespace MIS
{

    /**
     * @brief      This class describes an 3D object using Qt signals and slots.
     */
    class ENGINE3D_EXPORT Object3DQt : public QObject, public Object3D
    {
        Q_OBJECT

    public:
        Object3DQt(QObject* parent = nullptr);

    public slots:
        //TRANSFORMS
        void translate(const vec3& _translation, const bool& _onGround = false);
        void translate(const float& tx, const float& ty, const float& tz, const bool& onGround = false);
        void rotate(const float& _angle, const vec3& _axis, const bool& _verticalAxis = false);
        void rotate(const mat4& _rotation, const bool& _verticalAxis = false);
        void rotateXYZ(const vec3& rotation, const bool& verticalAxis = false);
        void rotateYXZ(const vec3& rotation, const bool& verticalAxis = false);
        void rotateZYX(const vec3& rotation, const bool& verticalAxis = false);
        void yaw(const float& _yaw);
        void pitch(const float& _pitch);
        void roll(const float& _roll);

        //SETTERS
        void setPose(const mat4& _pose);
        void setPosition(const vec3& _position);
        void setPosition(const float& tx, const float& ty, const float& tz);
        void setPositionX(const float& _tx);
        void setPositionY(const float& _ty);
        void setPositionZ(const float& _tz);
        void setPositionX(const double& _tx);
        void setPositionY(const double& _ty);
        void setPositionZ(const double& _tz);
        void setRotationXYZ(vec3 _rotation);
        void setRotation(mat4 _rotation);
        void setRotationX(const float& _rx);
        void setRotationY(const float& _ry);
        void setRotationZ(const float& _rz);
        void setRotationX(const double& _rx);
        void setRotationY(const double& _ry);
        void setRotationZ(const double& _rz);
        void setYawPitchRoll(const float& _yaw, const float& _pitch, const float& _roll);
        void setYaw(const float& _yaw);
        void setPitch(const float& _pitch);
        void setRoll(const float& _roll);
        void setYaw(const double& _yaw);
        void setPitch(const double& _pitch);
        void setRoll(const double& _roll);

    signals:
        void objectMoved();
    };

    glm::mat4 ENGINE3D_EXPORT stringToMat4(QString matString);
    glm::vec3 ENGINE3D_EXPORT stringToVec3(QString vecString);

}

#endif // OBJECT3DQT_H
