#include "TrajectoryManager.h"

namespace MIS
{

    TrajectoryManager::TrajectoryManager()
        : fps(25)
    {

    }

    TrajectoryManager::~TrajectoryManager()
    {
        for (CameraPose* pose : poses) delete pose;
    }

    void TrajectoryManager::addPose(Camera& camera, unsigned int delay, bool verticalAxis, bool useYawPitchRoll)
    {
        poses.append(new CameraPose(camera, delay, verticalAxis, useYawPitchRoll));
    }

    void TrajectoryManager::addPose(QString name, mat4 pose, float fov, unsigned int delay, float zNear, float zFar, bool verticalAxis, bool useYawPitchRoll)
    {
        poses.append(new CameraPose(name, pose, fov, delay, zNear, zFar, verticalAxis, useYawPitchRoll));
    }

    void TrajectoryManager::addPose(const CameraPose& pose)
    {
        poses.append(new CameraPose(pose));
    }

    TrajectoryManager::CameraPose* TrajectoryManager::getPose(unsigned int index)
    {
        return poses[index];
    }

    QVector<TrajectoryManager::CameraPose*>& TrajectoryManager::getPoses()
    {
        return poses;
    }

    void TrajectoryManager::removePose(unsigned int index)
    {
        delete poses[index];
        poses.removeAt(index);
    }

    void TrajectoryManager::setFPS(unsigned int _fps)
    {
        fps = _fps;
    }

    unsigned int TrajectoryManager::getFPS()
    {
        return fps;
    }

    unsigned int TrajectoryManager::getFrameNumber(unsigned int index)
    {
        if (index == 0) return 0;
        else return fps * poses[index]->delay / 1000.0f;
    }

    unsigned int TrajectoryManager::getFrameNumberSinceBegin(unsigned int index)
    {
        unsigned int number = 0;
        for (unsigned int i = 1; i <= index; i++)
        {
            number += getFrameNumber(i);
        }
        return number;
    }

    unsigned int TrajectoryManager::getTotalFrameNumber()
    {
        unsigned int totalFrameNumber = 0;
        for (unsigned int i = 1; i < poses.count(); i++)
        {
            totalFrameNumber += fps * poses[i]->delay / 1000;
        }
        return totalFrameNumber;
    }

    TrajectoryManager::CameraPose TrajectoryManager::getFrame(unsigned int frame)
    {
        CameraPose currentPose;

        unsigned int local_frame = frame;
        unsigned int frame_number = getFrameNumber(1);
        unsigned int prev_pose, next_pose = 1;

        for (unsigned int i = 2; i < poses.count() && frame > frame_number; i++)
        {
            local_frame -= getFrameNumber(i - 1);
            frame_number += getFrameNumber(i);
            next_pose++;
        }
        prev_pose = next_pose - 1;
        currentPose = *poses[next_pose];

        Object3D start, current, stop;
        start.setPose(poses[prev_pose]->pose);
        current.setPose(poses[prev_pose]->pose);
        stop.setPose(poses[next_pose]->pose);

        current.setPositionX(start.getPosition().x + local_frame * (stop.getPosition().x - start.getPosition().x) / getFrameNumber(next_pose));
        current.setPositionY(start.getPosition().y + local_frame * (stop.getPosition().y - start.getPosition().y) / getFrameNumber(next_pose));
        current.setPositionZ(start.getPosition().z + local_frame * (stop.getPosition().z - start.getPosition().z) / getFrameNumber(next_pose));

        if (poses[next_pose]->useYawPitchRoll)
        {
            current.setYawPitchRoll(
                start.getYaw() + local_frame * (stop.getYaw() - start.getYaw()) / getFrameNumber(next_pose),
                start.getPitch() + local_frame * (stop.getPitch() - start.getPitch()) / getFrameNumber(next_pose),
                start.getRoll() + local_frame * (stop.getRoll() - start.getRoll()) / getFrameNumber(next_pose)
            );
        }
        else
        {
            current.setRotationX(start.getRotation().x + local_frame * (stop.getRotation().x - start.getRotation().x) / getFrameNumber(next_pose));
            current.setRotationY(start.getRotation().y + local_frame * (stop.getRotation().y - start.getRotation().y) / getFrameNumber(next_pose));
            current.setRotationZ(start.getRotation().z + local_frame * (stop.getRotation().z - start.getRotation().z) / getFrameNumber(next_pose));
        }

        unsigned int frameNumber = getFrameNumber(next_pose);

        currentPose.pose = current.getPose();
        currentPose.zNear = poses[prev_pose]->zNear + local_frame * (poses[next_pose]->zNear - poses[prev_pose]->zNear) / getFrameNumber(next_pose);
        currentPose.zFar = poses[prev_pose]->zFar + local_frame * (poses[next_pose]->zFar - poses[prev_pose]->zFar) / getFrameNumber(next_pose);
        currentPose.fov = poses[prev_pose]->fov + local_frame * (poses[next_pose]->fov - poses[prev_pose]->fov) / getFrameNumber(next_pose);
        currentPose.delay = currentPose.delay / getFrameNumber(next_pose);

        return currentPose;
    }

}