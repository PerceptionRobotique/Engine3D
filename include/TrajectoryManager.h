#ifndef TRAJECTORYMANAGER_H
#define TRAJECTORYMANAGER_H

#include <QVector>

#include "Camera.h"
#include "Object3D.h"

namespace MIS
{

    class ENGINE3D_EXPORT TrajectoryManager
    {
    public:
        struct CameraPose
        {
            QString name;
            mat4 pose;
            float zNear, zFar;
            float fov;
            unsigned int delay;
            bool verticalAxis;
            bool useYawPitchRoll;

            CameraPose(QString _name = "empty", mat4 _pose = mat4(1.0), float _fov = 90.0f, unsigned int _delay = 3000, float _zNear = 0.1f, float _zFar = 500.0f, bool _verticalAxis = false, bool _useYawPitchRoll = false)
            {
                name = _name;
                pose = _pose;
                fov = _fov;
                delay = _delay;
                zNear = _zNear;
                zFar = _zFar;
                verticalAxis = _verticalAxis;
                useYawPitchRoll = _useYawPitchRoll;
            };

            CameraPose(const CameraPose& cameraPose)
            {
                name = cameraPose.name;
                pose = cameraPose.pose;
                fov = cameraPose.fov;
                delay = cameraPose.delay;
                zNear = cameraPose.zNear;
                zFar = cameraPose.zFar;
                verticalAxis = cameraPose.verticalAxis;
                useYawPitchRoll = cameraPose.useYawPitchRoll;
            };

            CameraPose& operator=(const CameraPose& cameraPose)
            {
                name = cameraPose.name;
                pose = cameraPose.pose;
                fov = cameraPose.fov;
                delay = cameraPose.delay;
                zNear = cameraPose.zNear;
                zFar = cameraPose.zFar;
                verticalAxis = cameraPose.verticalAxis;
                useYawPitchRoll = cameraPose.useYawPitchRoll;
                return *this;
            };

            CameraPose(Camera& camera, unsigned int _delay = 5000, bool _verticalAxis = false, bool _useYawPitchRoll = false, QVector<bool> _invert = { false, false, false })
            {
                vec3 position = camera.getPosition();
                vec3 rotation = camera.getRotation();

                name = "(" + QString::number(position.x) + " ; " + QString::number(position.y) + " ; " + QString::number(position.z) + ")  " +
                    "(" + QString::number(rotation.x) + " ; " + QString::number(rotation.y) + " ; " + QString::number(rotation.z) + ")";
                pose = camera.getwMc();
                fov = camera.getFOV();
                delay = _delay;
                zNear = camera.getNearPlane();
                zFar = camera.getFarPlane();
                verticalAxis = _verticalAxis;
                useYawPitchRoll = _useYawPitchRoll;
            };
        };

        TrajectoryManager();
        ~TrajectoryManager();

        void addPose(Camera& camera, unsigned int delay = 1000, bool verticalAxis = false, bool useYawPitchRoll = false);
        void addPose(QString name = "empty", mat4 pose = mat4(1.0), float fov = 90.0f, unsigned int delay = 3000, float zNear = 0.1f, float zFar = 500.0f, bool verticalAxis = false, bool useYawPitchRoll = false);
        void addPose(const CameraPose& pose);
        CameraPose* getPose(unsigned int index);
        QVector<CameraPose*>& getPoses();
        void removePose(unsigned int index);

        void setFPS(unsigned int _fps);
        unsigned int getFPS();

        unsigned int getFrameNumber(unsigned int index);
        unsigned int getFrameNumberSinceBegin(unsigned int index);
        unsigned int getTotalFrameNumber();
        CameraPose getFrame(unsigned int frame);

    private:
        QVector<CameraPose*> poses;
        unsigned int fps;
    };

}

#endif // TRAJECTORYMANAGER_H
