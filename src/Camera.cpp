#include "Camera.h"

namespace MIS
{

    Camera::Camera()
        : active(true)
        , target(vec3(0, 0, 0))
        , size(1920, 1080)
        , FBO(nullptr)
        , textureFBO(nullptr)
        , backgroundColor(0, 0, 0, 255)
        , samples(0)
        , projectionType(PERSPECTIVE)
        , viewPoint(FIRST_PERSON_VIEW)
        , nearPlane(0.1f)
        , farPlane(500.0f)
        , au(540)
        , av(540)
        , ku(30)
        , kv(30)
        , u0(size.width() / 2.0f)
        , v0(size.height() / 2.0f)
    {
        connect(this, SIGNAL(objectChanged()), this, SIGNAL(cameraChanged()));
    }

    Camera::~Camera()
    {
        if (textureFBO) delete textureFBO;
        if (FBO) delete FBO;
    }

    QOpenGLFramebufferObject* Camera::getFBO() const
    {
        return FBO;
    }

    bool Camera::bind()
    {
        if (active)
        {
            if (FBO == nullptr)
            {
                QOpenGLFramebufferObjectFormat FBO_Format;
                FBO_Format.setAttachment(QOpenGLFramebufferObject::Depth);
                FBO_Format.setSamples(samples);
                FBO = new QOpenGLFramebufferObject(size, FBO_Format);
            }
            return FBO->bind();
        }
        else return false;
    }

    void Camera::release() const
    {
        FBO->release();
    }

    GLuint Camera::texture()
    {
        if (!textureFBO)
        {
            textureFBO = new QOpenGLFramebufferObject(size);
            textureFBO->release();
        }

        QOpenGLFramebufferObject::blitFramebuffer(textureFBO, FBO, GL_COLOR_BUFFER_BIT);
        return textureFBO->texture();
    }

    QImage Camera::toImage() const
    {
        QImage image = FBO->toImage();
        return image;
    }

    bool Camera::isActive() const
    {
        return active;
    }

    QSize Camera::getSize() const
    {
        return size;
    }

    int Camera::getWidth() const
    {
        return size.width();
    }

    int Camera::getHeight() const
    {
        return size.height();
    }

    void Camera::setActive(bool _active)
    {
        active = _active;
    }

    void Camera::setSize(QSize _size)
    {
        size = _size;
        u0 = size.width() / 2.0f;
        v0 = size.height() / 2.0f;

        if (FBO)
        {
            delete FBO;
            FBO = nullptr;
        }

        if (textureFBO)
        {
            delete textureFBO;
            textureFBO = nullptr;
        }

        emit sizeChanged();
        emit cameraChanged();
    }

    void Camera::setSize(int _width, int _height)
    {
        setSize(QSize(_width, _height));
    }

    void Camera::setWidth(int _width)
    {
        setSize(_width, size.height());
    }

    void Camera::setHeight(int _height)
    {
        setSize(size.width(), _height);
    }

    void Camera::setBackgroundColor(QColor _backgroundColor)
    {
        backgroundColor = _backgroundColor;
    }

    void Camera::setBackgroundColor(float red, float green, float blue, float alpha)
    {
        backgroundColor = QColor(red * 255.0f, green * 255.0f, blue * 255.0f, alpha * 255.0f);
    }

    void Camera::setSamples(int _samples)
    {
        samples = _samples;
        if (FBO)
        {
            delete FBO;
            FBO = nullptr;
        }
        emit cameraChanged();
    }

    void Camera::setTarget(vec3 _target)
    {
        target = _target;
        lookAt(target);
        emit cameraChanged();
    }

    void Camera::setTargetX(double x)
    {
        setTarget(vec3(x, target.y, target.z));
    }

    void Camera::setTargetY(double y)
    {
        setTarget(vec3(target.x, y, target.z));
    }

    void Camera::setTargetZ(double z)
    {
        setTarget(vec3(target.x, target.y, z));
    }

    void Camera::setNearPlane(float _nearPlane)
    {
        nearPlane = _nearPlane;
        emit cameraChanged();
    }

    void Camera::setFarPlane(float _farPlane)
    {
        farPlane = _farPlane;
        emit cameraChanged();
    }

    void Camera::setProjectionType(ProjectionType _projectionType)
    {
        projectionType = _projectionType;
        emit cameraChanged();
    }

    void Camera::setCustomProjection(mat4 _customProjection)
    {
        customProjection = _customProjection;
        if (projectionType == CUSTOM)
            emit cameraChanged();
    }

    void Camera::setViewPoint(ViewPoint _viewPoint)
    {
        viewPoint = _viewPoint;
        if (viewPoint == THIRD_PERSON_VIEW) lookAt(target);
        emit cameraChanged();
    }

    void Camera::setcMw(mat4 _cMw)
    {
        setPose(glm::inverse(_cMw));
    }

    void Camera::setwMc(mat4 _wMc)
    {
        setPose(_wMc);
    }

    void Camera::lookAt(vec3 _point)
    {
        vec3 position = getPosition();
        setcMw(glm::lookAt(getPosition(), _point, vec3(0, 1, 0)));
        setPosition(position);
        if (viewPoint == THIRD_PERSON_VIEW) target = _point;
    }

    void Camera::lookAt(Model3D* model)
    {
        lookAt((model->getwMo() * model->getAABB()).center);
    }

    void Camera::setView(Model3D* model, Camera::View view)
    {
        if (model)
        {
            switch (view)
            {
            case FRONT:
            {
                vec3 center = (model->getwMo() * model->getAABB()).center;
                float distance = glm::distance(center, getPosition());
                setPosition(center);
                setRotation(vec3(0, 0, 0));
                translate(vec3(0, 0, distance));
                setTarget(center);
                break;
            }

            case BACK:
            {
                vec3 center = (model->getwMo() * model->getAABB()).center;
                float distance = glm::distance(center, getPosition());
                setPosition(center);
                setRotation(vec3(0, 180, 0));
                translate(vec3(0, 0, distance));
                setTarget(center);
                break;
            }

            case TOP:
            {
                vec3 center = (model->getwMo() * model->getAABB()).center;
                float distance = glm::distance(center, getPosition());
                setPosition(vec3(center.x, center.y + distance, center.z));
                setRotation(vec3(-90, 0, 0));
                setTarget(center);
                break;
            }

            case BOTTOM:
            {
                vec3 center = (model->getwMo() * model->getAABB()).center;
                float distance = glm::distance(center, getPosition());
                setPosition(vec3(center.x, center.y - distance, center.z));
                setRotation(vec3(90, 0, 0));
                setTarget(center);
                break;
            }

            case LEFT:
            {
                vec3 center = (model->getwMo() * model->getAABB()).center;
                float distance = glm::distance(center, getPosition());
                setPosition(center);
                setRotation(vec3(0, -90, 0));
                translate(vec3(0, 0, distance));
                setTarget(center);
                break;
            }

            case RIGHT:
            {
                vec3 center = (model->getwMo() * model->getAABB()).center;
                float distance = glm::distance(center, getPosition());
                setPosition(center);
                setRotation(vec3(0, 90, 0));
                translate(vec3(0, 0, distance));
                setTarget(center);
                break;
            }

            case CENTER:
            {
                lookAt(model);
                if (getProjectionType() != Camera::EQUIRECTANGULAR)
                {
                    QVector<glm::vec3> box = model->getBox();
                    vec4 point(0, 0, 0, 1.0f);
                    vec3 point2D(0, 0, 0);
                    float currentMax = 0;
                    for (int i = 0; i < box.count(); i++)
                    {
                        vec3 _point2D = getProjection() * getcMw() * model->getwMo() * vec4(box[i].x, box[i].y, box[i].z, 1);
                        if (max(abs(_point2D.x), abs(_point2D.y)) > currentMax || i == 0)
                        {
                            currentMax = max(abs(_point2D.x), abs(_point2D.y));
                            point2D = _point2D;
                            point = getcMw() * model->getwMo() * vec4(box[i].x, box[i].y, box[i].z, 1);
                        }
                    }
                    float factor = point.y;
                    float fov = getFOV();
                    if (abs(point2D.x) > abs(point2D.y))
                    {
                        factor = point.x;
                        fov = getHFOV();
                    }
                    if (getProjectionType() == Camera::ORTHOGRAPHIC)
                    {
                        fov = 180;
                    }
                    float d = factor / sin(radians(fov / 2.0f)) * cos(radians(fov / 2.0f));
                    translate(vec3(0, 0, point.z + abs(d)));
                }
                break;
            }
            }
        }
    }

    mat4 Camera::getProjection() const
    {
        mat4 projection;
        switch (projectionType)
        {
        case PERSPECTIVE:
            projection = mat4(
                2.0f * au / size.width(), 0.0f, 0.0f, 0.0f,
                0.0f, (2.0f * av / size.height()), 0.0f, 0.0f,
                -(2.0f * (u0 / size.width()) - 1.0f), -(2.0f * ((size.height() - v0) / size.height()) - 1.0f), (-1.0f * (farPlane + nearPlane) / (farPlane - nearPlane)), -1.0f,
                0.0f, 0.0f, (-2.0f * farPlane * nearPlane / (farPlane - nearPlane)), 0.0f
            );
            break;

        case ORTHOGRAPHIC:
        {
            float aspectRatio = getAspectRatio();
            float BM = distance(getPosition(), target) * size.width() / (2 * au);

            float XM = BM;
            float Xm = -BM;
            float YM = BM / aspectRatio;
            float Ym = -BM / aspectRatio;

            projection = mat4(
                (2.0f / (XM - Xm)), 0.0f, 0.0f, 0.0f,
                0.0f, (2.0f / (YM - Ym)), 0.0f, 0.0f,
                0.0f, 0.0f, (-2.0f / (farPlane - nearPlane)), 0.0f,
                -(XM + Xm) / (XM - Xm), -(YM + Ym) / (YM - Ym), -(farPlane + nearPlane) / (farPlane - nearPlane), 1.0f
            );
            break;
        }

        case EQUIRECTANGULAR:
        {
            constexpr float arg1122 = 1.0f / pi<float>();
            projection = mat4(
                arg1122, 0.0f, 0.0f, 0.0f,
                0.0f, arg1122, 0.0f, 0.0f,
                0.0f, 0.0f, (-2.0f / (farPlane - nearPlane)), 0.0f,
                0.0f, 0.0f, -(farPlane + nearPlane) / (farPlane - nearPlane), 1.0f
            );
            break;
        }

        case CUSTOM:
            projection = customProjection;
            break;
        }
        return projection;
    }

    float* Camera::getProjectionPtr()
    {
        proj = getProjection();
        return value_ptr(proj);
    }

    mat4 Camera::getcMw() const
    {
        return inverse(getPose());
    }

    float* Camera::getcMwPtr()
    {
        view = inverse(getPose());
        return value_ptr(view);
    }

    mat4 Camera::getwMc() const
    {
        return getPose();
    }

    bool Camera::cullingTest(const Model3D* model) const
    {
        if (projectionType == EQUIRECTANGULAR)
            return true;
        else
        {
            QVector<glm::vec3> box = model->getBox();
            QVector<vec4> points;
            float minZ = 0.0f, maxZ = 0.0f;
            for (int i = 0; i < box.count(); i++)
            {
                float z = -vec4(getcMw() * model->getwMo() * vec4(box[i].x, box[i].y, box[i].z, 1)).z;
                if (minZ > z || i == 0) minZ = z;
                if (maxZ < z || i == 0) maxZ = z;
            }
            for (int i = 0; i < box.count(); i++)
            {
                float z = vec4(getcMw() * model->getwMo() * vec4(box[i].x, box[i].y, box[i].z, 1)).z;
                if ((-z >= nearPlane && -z <= farPlane) || (minZ <= nearPlane && maxZ >= farPlane))
                {
                    points.append(getProjection() * getcMw() * model->getwMo() * vec4(box[i].x, box[i].y, box[i].z, 1));
                    points.last() /= points.last().w;
                }
            }

            if (!points.isEmpty())
            {
                vec4 min = points[0];
                vec4 max = points[0];
                for (int i = 1; i < points.count(); i++)
                {
                    if (min.x > points[i].x) min.x = points[i].x;
                    if (min.y > points[i].y) min.y = points[i].y;
                    if (min.z > points[i].z) min.z = points[i].z;

                    if (max.x < points[i].x) max.x = points[i].x;
                    if (max.y < points[i].y) max.y = points[i].y;
                    if (max.z < points[i].z) max.z = points[i].z;
                }

                float limit = 1.0f;

                return (min.x >= -limit && min.x <= limit && min.y >= -limit && min.y <= limit) ||
                    (min.x >= -limit && min.x <= limit && max.y >= -limit && max.y <= limit) ||
                    (max.x >= -limit && max.x <= limit && min.y >= -limit && min.y <= limit) ||
                    (max.x >= -limit && max.x <= limit && max.y >= -limit && max.y <= limit) ||
                    (min.y <= -limit && max.y >= limit && ((min.x >= -limit && min.x <= limit) || (max.x >= -limit && max.x <= limit))) ||
                    (min.x <= -limit && max.x >= limit && ((min.y >= -limit && min.y <= limit) || (max.y >= -limit && max.y <= limit))) ||
                    (min.x <= -limit && max.x >= limit && min.y <= -limit && max.y >= limit)
                    ;
            }
            else
                return false;
        }
    }

    float Camera::distanceWith(const Model3D* model) const
    {
        return glm::distance(getPosition(), (model->getwMo() * model->getAABB()).center);
    }

    void Camera::translate(const vec3& _translation, const bool& _onGround)
    {
        switch (viewPoint)
        {
        case FIRST_PERSON_VIEW:
            Object3DQt::translate(_translation, _onGround);
            target = glm::translate(getwMc(), vec3(0, 0, -1))[3];
            break;

        case THIRD_PERSON_VIEW:
            if (_translation.z < 0 && abs(_translation.z) > glm::distance(getPosition(), target))
                Object3DQt::translate(vec3(_translation.x, _translation.y, -glm::distance(getPosition(), target) + 1), _onGround);
            else
                Object3DQt::translate(_translation, _onGround);
            float dist = glm::distance(getPosition(), target);
            target = glm::translate(getwMc(), vec3(0, 0, -dist))[3];
            break;
        }
    }

    void Camera::rotate(const float& _angle, const vec3& _axis, const bool& _verticalAxis)
    {
        switch (viewPoint)
        {
        case FIRST_PERSON_VIEW:
            Object3DQt::rotate(_angle, _axis, _verticalAxis);
            target = glm::translate(getwMc(), vec3(0, 0, -1))[3];
            break;

        case THIRD_PERSON_VIEW:
            float dist = glm::distance(getPosition(), target);
            Object3DQt::translate(vec3(0, 0, -dist));
            Object3DQt::rotate(_angle, _axis, _verticalAxis);
            Object3DQt::translate(vec3(0, 0, dist));
            float roll = getRoll();
            lookAt(target);
            Object3DQt::setRoll(roll);
            break;
        }
    }

    void Camera::rotate(const mat4& _rotation, const bool& _verticalAxis)
    {
        switch (viewPoint)
        {
        case FIRST_PERSON_VIEW:
            Object3DQt::rotate(_rotation, _verticalAxis);
            target = glm::translate(getwMc(), vec3(0, 0, -1))[3];
            break;

        case THIRD_PERSON_VIEW:
            float dist = glm::distance(getPosition(), target);
            Object3DQt::translate(vec3(0, 0, -dist));
            Object3DQt::rotate(_rotation, _verticalAxis);
            Object3DQt::translate(vec3(0, 0, dist));
            float roll = getRoll();
            lookAt(target);
            Object3DQt::setRoll(roll);
            break;
        }
    }

    void Camera::yaw(const float& _yaw)
    {
        switch (viewPoint)
        {
        case FIRST_PERSON_VIEW:
            Object3DQt::yaw(_yaw);
            target = glm::translate(getwMc(), vec3(0, 0, -1))[3];
            break;

        case THIRD_PERSON_VIEW:
            float dist = glm::distance(getPosition(), target);
            Object3DQt::translate(vec3(0, 0, -dist));
            Object3DQt::yaw(_yaw);
            Object3DQt::translate(vec3(0, 0, dist));
            float roll = getRoll();
            lookAt(target);
            Object3DQt::setRoll(roll);
            break;
        }
    }

    void Camera::pitch(const float& _pitch)
    {
        switch (viewPoint)
        {
        case FIRST_PERSON_VIEW:
            Object3DQt::pitch(_pitch);
            target = glm::translate(getwMc(), vec3(0, 0, -1))[3];
            break;

        case THIRD_PERSON_VIEW:
            float dist = glm::distance(getPosition(), target);
            Object3DQt::translate(vec3(0, 0, -dist));
            Object3DQt::pitch(_pitch);
            Object3DQt::translate(vec3(0, 0, dist));
            float roll = getRoll();
            lookAt(target);
            Object3DQt::setRoll(roll);
            break;
        }
    }

    void Camera::roll(const float& _roll)
    {
        switch (viewPoint)
        {
        case FIRST_PERSON_VIEW:
            Object3DQt::roll(_roll);
            target = glm::translate(getwMc(), vec3(0, 0, -1))[3];
            break;

        case THIRD_PERSON_VIEW:
            float dist = glm::distance(getPosition(), target);
            Object3DQt::translate(vec3(0, 0, -dist));
            Object3DQt::roll(_roll);
            Object3DQt::translate(vec3(0, 0, dist));
            float roll = getRoll();
            lookAt(target);
            Object3DQt::setRoll(roll);
            break;
        }
    }

    void Camera::setPose(const mat4& _pose)
    {
        Object3DQt::setPose(_pose);
        target = glm::translate(getwMc(), vec3(0, 0, -1))[3];
    }

    void Camera::setPosition(const vec3& _position)
    {
        Object3DQt::setPosition(_position);
        target = glm::translate(getwMc(), vec3(0, 0, -1))[3];
    }

    void Camera::setPositionX(const float& _tx)
    {
        Object3DQt::setPositionX(_tx);
        target = glm::translate(getwMc(), vec3(0, 0, -1))[3];
    }

    void Camera::setPositionY(const float& _ty)
    {
        Object3DQt::setPositionY(_ty);
        target = glm::translate(getwMc(), vec3(0, 0, -1))[3];
    }

    void Camera::setPositionZ(const float& _tz)
    {
        Object3DQt::setPositionZ(_tz);
        target = glm::translate(getwMc(), vec3(0, 0, -1))[3];
    }

    void Camera::setRotation(vec3 _rotation)
    {
        Object3DQt::setRotation(_rotation);
        target = glm::translate(getwMc(), vec3(0, 0, -1))[3];
    }

    void Camera::setRotation(mat4 _rotation)
    {
        Object3DQt::setRotation(_rotation);
        target = glm::translate(getwMc(), vec3(0, 0, -1))[3];
    }

    void Camera::setRotationX(const float& _rx)
    {
        Object3DQt::setRotationX(_rx);
        target = glm::translate(getwMc(), vec3(0, 0, -1))[3];
    }

    void Camera::setRotationY(const float& _ry)
    {
        Object3DQt::setRotationY(_ry);
        target = glm::translate(getwMc(), vec3(0, 0, -1))[3];
    }

    void Camera::setRotationZ(const float& _rz)
    {
        Object3DQt::setRotationZ(_rz);
        target = glm::translate(getwMc(), vec3(0, 0, -1))[3];
    }

    void Camera::setYawPitchRoll(const float& _yaw, const float& _pitch, const float& _roll)
    {
        Object3DQt::setYawPitchRoll(_yaw, _pitch, _roll);
        target = glm::translate(getwMc(), vec3(0, 0, -1))[3];
    }

    void Camera::setYaw(const float& _yaw)
    {
        Object3DQt::setYaw(_yaw);
        target = glm::translate(getwMc(), vec3(0, 0, -1))[3];
    }

    void Camera::setPitch(const float& _pitch)
    {
        Object3DQt::setPitch(_pitch);
        target = glm::translate(getwMc(), vec3(0, 0, -1))[3];
    }

    void Camera::setRoll(const float& _roll)
    {
        Object3DQt::setRoll(_roll);
        target = glm::translate(getwMc(), vec3(0, 0, -1))[3];
    }

    QColor Camera::getBackgroundColor() const
    {
        return backgroundColor;
    }

    vec3 Camera::getTarget() const
    {
        return target;
    }

    float Camera::getAspectRatio() const
    {
        return (float)size.width() / size.height();
    }

    float Camera::getNearPlane() const
    {
        return nearPlane;
    }

    float Camera::getFarPlane() const
    {
        return farPlane;
    }

    float Camera::getAu() const
    {
        return au;
    }

    void Camera::setAu(double _au)
    {
        au = _au;
        emit cameraChanged();
    }

    float Camera::getAv() const
    {
        return av;
    }

    void Camera::setAv(double _av)
    {
        av = _av;
        emit cameraChanged();
    }

    float Camera::getKu() const
    {
        return ku;
    }

    void Camera::setKu(double _ku)
    {
        ku = _ku;
        emit cameraChanged();
    }

    float Camera::getKv() const
    {
        return kv;
    }

    void Camera::setKv(double _kv)
    {
        kv = _kv;
        emit cameraChanged();
    }

    float Camera::getU0() const
    {
        return u0;
    }

    void Camera::setU0(double _u0)
    {
        u0 = _u0;
        emit cameraChanged();
    }

    float Camera::getV0() const
    {
        return v0;
    }

    void Camera::setV0(double _v0)
    {
        v0 = _v0;
        emit cameraChanged();
    }

    void Camera::setNearPlane(double _nearPlane)
    {
        setNearPlane((float)_nearPlane);
    }

    void Camera::setFarPlane(double _farPlane)
    {
        setFarPlane((float)_farPlane);
    }

    float Camera::getFOV() const
    {
        return 2.0f * degrees(atan(size.height() / (2.0f * av)));
    }

    float Camera::getHFOV() const
    {
        return 2.0f * degrees(atan(size.width() / (2.0f * au)));
    }

    Camera::ProjectionType Camera::getProjectionType() const
    {
        return projectionType;
    }

    Camera::ViewPoint Camera::getViewPoint() const
    {
        return viewPoint;
    }

    void Camera::setFOV(float _fov)
    {
        setFOV((double)_fov);
    }

    void Camera::setFOV(double _fov)
    {
        float hfov = 2.0f * degrees(atan(tan(radians(_fov) / 2.0f) * getAspectRatio()));
        av = size.height() / (2.0f * tan(radians(_fov / 2.0f)));
        emit cameraChanged();
    }

    void Camera::setHFOV(float _hfov)
    {
        setHFOV((double)_hfov);
    }

    void Camera::setHFOV(double _hfov)
    {
        au = size.width() / (2.0f * tan(radians(_hfov / 2.0f)));
        emit cameraChanged();
    }

}