#include "Camera.h"

namespace MIS
{

    Camera::Camera()
        : active(true)
        , target(vec3(0, 0, -1))
        , size(1920, 1080)
        , FBO(nullptr)
        , textureFBO(nullptr)
        , backgroundColor(0, 0, 0, 255)
        , samples(0)
        , projectionType(PERSPECTIVE)
        , viewPoint(FIRST_PERSON_VIEW)
        , nearPlane(0.1f)
        , farPlane(1000.0f)
        , au(540)
        , av(540)
        , ku(30)
        , kv(30)
        , u0(size.width() / 2.0f)
        , v0(size.height() / 2.0f)
    {
        connect(this, SIGNAL(objectMoved()), this, SIGNAL(cameraMoved()));
    }

    Camera::~Camera()
    {
        if (textureFBO) delete textureFBO;
        FBOMutex.lock();
        if (FBO) delete FBO;
        FBOMutex.unlock();
    }

    /// <summary>
    /// Get current camera framebuffer object.
    /// </summary>
    /// <returns>Framebuffer object. May be nullptr if camera size or samples changed.</returns>
    QOpenGLFramebufferObject* Camera::getFBO() const
    {
        return FBO;
    }

    bool Camera::bind()
    {
        if (active)
        {
            FBOMutex.lock();
            if (FBO)
            {
                if (FBO->size() != getSize())
                {
                    delete FBO;
                    FBO = nullptr;
                }
            }

            if (!FBO)
            {
                QOpenGLFramebufferObjectFormat FBO_Format;
                FBO_Format.setAttachment(QOpenGLFramebufferObject::Depth);
                FBO_Format.setSamples(samples);
                FBO = new QOpenGLFramebufferObject(size, FBO_Format);
                FBO->release();
            }
            return FBO->bind();
        }
        else return false;
    }

    void Camera::release()
    {
        FBO->release();
        FBOMutex.unlock();
    }

    GLuint Camera::texture()
    {
        if (samples > 0)
        {
            if (textureFBO)
            {
                if (textureFBO->size() != FBO->size())
                {
                    delete textureFBO;
                    textureFBO = nullptr;
                }
            }

            if (!textureFBO)
            {
                textureFBO = new QOpenGLFramebufferObject(size);
                textureFBO->release();
            }

            FBOMutex.lock();
            QOpenGLFramebufferObject::blitFramebuffer(textureFBO, FBO, GL_COLOR_BUFFER_BIT);
            FBOMutex.unlock();
            return textureFBO->texture();
        }
        else return FBO->texture();
    }

    QImage Camera::toImage()
    {
        FBOMutex.lock();
        if (FBO) frame = FBO->toImage();
        FBOMutex.unlock();
        return frame;
    }

    QImage Camera::getFrame()
    {
        FBOMutex.lock();
        QImage copy = frame;
        FBOMutex.unlock();
        return copy;
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
        float f = getFOV();
        size = _size;
        setFOV(f);
        setAu(getAv());
        u0 = size.width() / 2.0f;
        v0 = size.height() / 2.0f;

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
        emit cameraChanged();
    }

    void Camera::setBackgroundColor(float red, float green, float blue, float alpha)
    {
        backgroundColor = QColor(red * 255.0f, green * 255.0f, blue * 255.0f, alpha * 255.0f);
    }

    void Camera::setSamples(int _samples)
    {
        samples = _samples;
        FBOMutex.lock();
        if (FBO)
        {
            delete FBO;
            FBO = nullptr;
        }
        FBOMutex.unlock();
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
        if (distance(getPosition(), _point) > 0)
        {
            vec3 position = getPosition();
            vec3 vAxis(0, 1, 0);
            if (abs(getRotation().x) == 90) vAxis = vec3(0, 0, -1);
            mat4 pose = glm::lookAt(getPosition(), _point, vAxis);
            if (transpose(pose)[3] == vec4(0, 0, 0, 1) && !QString::fromStdString(to_string(pose)).contains("nan"))
            {
                setcMw(pose);
                setPosition(position);
                if (viewPoint == THIRD_PERSON_VIEW) target = _point;
            }
            else qDebug() << "invalid matrix : " << QString::fromStdString(to_string(pose));
        }
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
                if (distance == 0) distance = 1;
                setPosition(center);
                setRotationXYZ(vec3(0, 0, 0));
                translate(vec3(0, 0, distance));
                setTarget(center);
                break;
            }

            case BACK:
            {
                vec3 center = (model->getwMo() * model->getAABB()).center;
                float distance = glm::distance(center, getPosition());
                if (distance == 0) distance = 1;
                setPosition(center);
                setRotationXYZ(vec3(0, 180, 0));
                translate(vec3(0, 0, distance));
                setTarget(center);
                break;
            }

            case TOP:
            {
                vec3 center = (model->getwMo() * model->getAABB()).center;
                float distance = glm::distance(center, getPosition());
                if (distance == 0) distance = 1;
                setPosition(vec3(center.x, center.y + distance, center.z));
                setRotationXYZ(vec3(-90, 0, 0));
                setTarget(center);
                break;
            }

            case BOTTOM:
            {
                vec3 center = (model->getwMo() * model->getAABB()).center;
                float distance = glm::distance(center, getPosition());
                if (distance == 0) distance = 1;
                setPosition(vec3(center.x, center.y - distance, center.z));
                setRotationXYZ(vec3(90, 0, 0));
                setTarget(center);
                break;
            }

            case LEFT:
            {
                vec3 center = (model->getwMo() * model->getAABB()).center;
                float distance = glm::distance(center, getPosition());
                if (distance == 0) distance = 1;
                setPosition(center);
                setRotationXYZ(vec3(0, -90, 0));
                translate(vec3(0, 0, distance));
                setTarget(center);
                break;
            }

            case RIGHT:
            {
                vec3 center = (model->getwMo() * model->getAABB()).center;
                float distance = glm::distance(center, getPosition());
                if (distance == 0) distance = 1;
                setPosition(center);
                setRotationXYZ(vec3(0, 90, 0));
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
                        vec4 _point2D = getiMc() * getcMw() * model->getwMo() * vec4(box[i], 1.0);
                        _point2D /= _point2D.w;
                        if (max(abs(_point2D.x), abs(_point2D.y)) > currentMax || i == 0)
                        {
                            currentMax = max(abs(_point2D.x), abs(_point2D.y));
                            point2D = _point2D;
                            point = getcMw() * model->getwMo() * vec4(box[i], 1.0);
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
                        fov = 180;
                    float d = factor / sin(radians(fov / 2.0f)) * cos(radians(fov / 2.0f));
                    translate(vec3(0, 0, point.z + abs(d)));
                }
                else
                {
                    setPosition(model->getwMo() * vec4(model->getAABB().center, 1.0));
                    setRotationXYZ(vec3(0, 0, 0));
                }
                break;
            }
            }
        }
    }

    void Camera::saveCameraParameters(const QString& fileName) const
    {
        QSettings cameraFile(fileName, QSettings::IniFormat);
        cameraFile.setValue("width", size.width());
        cameraFile.setValue("height", size.height());
        cameraFile.setValue("near", nearPlane);
        cameraFile.setValue("far", farPlane);
        cameraFile.setValue("ViewPoint", viewPoint);
        cameraFile.setValue("ProjectionType", projectionType);
        cameraFile.setValue("au", au);
        cameraFile.setValue("av", av);
        cameraFile.setValue("ku", ku);
        cameraFile.setValue("kv", kv);
        cameraFile.setValue("u0", u0);
        cameraFile.setValue("v0", v0);
        cameraFile.setValue("target", QString::fromStdString(to_string(target)));
    }

    void Camera::loadCameraParameters(const QString& fileName)
    {
        QSettings cameraFile(fileName, QSettings::IniFormat);
        setSize(cameraFile.value("width").toInt(), cameraFile.value("height").toInt());
        setNearPlane(cameraFile.value("near").toFloat());
        setFarPlane(cameraFile.value("far").toFloat());
        setViewPoint((ViewPoint)cameraFile.value("ViewPoint").toInt());
        setProjectionType((ProjectionType)cameraFile.value("ProjectionType").toInt());
        setAu(cameraFile.value("au").toFloat());
        setAv(cameraFile.value("av").toFloat());
        setKu(cameraFile.value("ku").toFloat());
        setKv(cameraFile.value("kv").toFloat());
        setU0(cameraFile.value("u0").toFloat());
        setV0(cameraFile.value("v0").toFloat());
        setTarget(stringToVec3(cameraFile.value("target").toString()));
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

    mat4 Camera::getiMc() const
    {
        return getProjection();
    }

    mat4 Camera::getcMi() const
    {
        return inverse(getiMc());
    }

    mat4 Camera::getcMw() const
    {
        return inverse(getPose());
    }

    mat4 Camera::getwMc() const
    {
        return getPose();
    }

    /// <summary>
    /// Test if a model is visible by a camera
    /// </summary>
    /// <param name="model">Model tested</param>
    /// <returns>Model is visible</returns>
    bool Camera::isModelVisible(const Model3D* model) const
    {
        if (isActive())
        {
            if (projectionType == EQUIRECTANGULAR)
                return true;
            else
            {
                if (model->isPointInAABB(getPosition()))
                    return true;
                else
                {
                    QVector<vec3> box = model->getBox();
                    QVector<vec4> boxImage;
                    for (vec3 point : box)
                    {
                        boxImage.append(getiMc() * getcMw() * model->getwMo() * vec4(point, 1.0));
                        boxImage.last() /= boxImage.last().w;
                        boxImage.last().z = -(getcMw() * model->getwMo() * vec4(point, 1.0)).z;
                    }
                    vec3 min2D = boxImage.first();
                    vec3 max2D = boxImage.first();
                    for (vec3 dot : boxImage)
                    {
                        min2D.x = min(min2D.x, dot.x);
                        min2D.y = min(min2D.y, dot.y);
                        min2D.z = min(min2D.z, dot.z);
                        max2D.x = max(max2D.x, dot.x);
                        max2D.y = max(max2D.y, dot.y);
                        max2D.z = max(max2D.z, dot.z);
                    }
                    return min2D.x <= 1 && min2D.y <= 1 && min2D.z <= getFarPlane() && max2D.x >= -1 && max2D.y >= -1 && max2D.z >= getNearPlane();
                }
            }
        }
        else return false;
    }

    float Camera::distanceWith(const Model3D* model) const
    {
        QVector<vec3> box = model->getBox();
        vec3 camPos = getPosition();
        Model3D::AABB modelAABB = model->getwMo() * model->getAABB();
        if (
            camPos.x >= modelAABB.min.x &&
            camPos.y >= modelAABB.min.y &&
            camPos.z >= modelAABB.min.z &&
            camPos.x <= modelAABB.max.x &&
            camPos.y <= modelAABB.max.y &&
            camPos.z <= modelAABB.max.z
            )
            return 0;
        else
        {
            float dist = glm::distance(camPos, modelAABB.center) - glm::distance(modelAABB.center, modelAABB.min);
            return dist;
        }
    }

    vec2 Camera::projectPoint(vec4 point3D) const
    {
        vec4 point2D = getiMc() * getcMw() * point3D;
        point2D /= point2D.w;
        return point2D;
    }
    
    vec2 Camera::projectPoint(vec3 point3D) const
    {
        return projectPoint(vec4(point3D, 1.0));
    }

    QVector<vec2> Camera::projectPoints(const QVector<vec4>& points3D) const
    {
        QVector<vec2> points2D;
        for (const vec4& point3D : points3D)
            points2D.append(projectPoint(point3D));
        return points2D;
    }

    QVector<vec2> Camera::projectPoints(const QVector<vec3>& points3D) const
    {
        QVector<vec2> points2D;
        for (const vec3& point3D : points3D)
            points2D.append(projectPoint(point3D));
        return points2D;
    }

    vec2 Camera::meterToPixel(vec2 point2D) const
    {
        vec2 pixel;
        pixel.x = point2D.x * (getWidth() - 1);
        pixel.y = point2D.y * (getHeight() - 1);
        return pixel;
    }

    QVector<vec2> Camera::meterToPixel(const QVector<vec2>& points2D) const
    {
        QVector<vec2> pixels;
        for (const vec2& point2D : points2D)
            pixels.append(meterToPixel(point2D));
        return pixels;
    }

    vec2 Camera::pixelToMeter(vec2 pixel) const
    {
        vec2 point2D;
        point2D.x = pixel.x / (getWidth() - 1);
        point2D.y = pixel.y / (getHeight() - 1);
        return point2D;
    }

    QVector<vec2> Camera::pixelToMeter(const QVector<vec2>& pixels) const
    {
        QVector<vec2> points2D;
        for (const vec2& pixel : pixels)
            points2D.append(pixelToMeter(pixel));
        return points2D;
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
            Object3D::translate(vec3(0, 0, -dist));
            Object3D::rotate(_angle, _axis, _verticalAxis);
            Object3D::translate(vec3(0, 0, dist));
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
            Object3D::translate(vec3(0, 0, -dist));
            Object3D::rotate(_rotation, _verticalAxis);
            Object3D::translate(vec3(0, 0, dist));
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
            Object3D::translate(vec3(0, 0, -dist));
            Object3D::yaw(_yaw);
            Object3D::translate(vec3(0, 0, dist));
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
            Object3D::translate(vec3(0, 0, -dist));
            Object3D::pitch(_pitch);
            Object3D::translate(vec3(0, 0, dist));
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
            Object3D::translate(vec3(0, 0, -dist));
            Object3D::roll(_roll);
            Object3D::translate(vec3(0, 0, dist));
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

    void Camera::setRotationXYZ(vec3 _rotation)
    {
        Object3DQt::setRotationXYZ(_rotation);
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

    unsigned int Camera::getSamples() const
    {
        return samples;
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
        setAu(av);
        emit cameraChanged();
    }

    void Camera::setVFOV(float _fov)
    {
        setFOV((double)_fov);
    }

    void Camera::setVFOV(double _fov)
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