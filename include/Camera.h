#ifndef CAMERA_H
#define CAMERA_H

#include <QObject>
#include <QSize>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFramebufferObjectFormat>
#include <QImage>
#include <QColor>
#include <QDebug>

#include <glm/common.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Object3DQt.h"
#include "Model3D.h"

using namespace glm;

class ENGINE3D_EXPORT Camera : public Object3DQt
{
    Q_OBJECT
public:
    enum ProjectionType{
        PERSPECTIVE,
        ORTHOGRAPHIC,
        EQUIRECTANGULAR,
        CUSTOM
    };

    enum ViewPoint{
        FIRST_PERSON_VIEW,
        THIRD_PERSON_VIEW
    };

    enum View{
        FRONT,
        BACK,
        LEFT,
        RIGHT,
        TOP,
        BOTTOM,
        CENTER
    };

    Camera();
    ~Camera();

    QOpenGLFramebufferObject* getFBO();
    bool bind();
    void release();
    GLuint texture();
    QImage toImage();
    void setSamples(unsigned int _samples);

    inline QSize getSize() { return size; };
    inline int getWidth() {return size.width(); };
    inline int getHeight() { return size.height(); };
    QColor getBackgroundColor() const;
    vec3 getViewCenter();
    float getAspectRatio();
    float getNearPlane();
    float getFarPlane();
    float getAu();
    float getAv();
    float getKu();
    float getKv();
    float getU0();
    float getV0();
    float getFOV();
    float getHFOV();
    inline Camera::ProjectionType getProjectionType() { return projectionType; };
    Camera::ViewPoint getViewPoint();
    mat4 getProjection();
    float* getProjectionPtr();
    mat4 getcMw();
    float* getcMwPtr();
    mat4 getwMc();
    bool cullingTest(Model3D* model);
    float distanceWith(Model3D* model);

public slots:
    //TRANSFORMS
    void translate(const vec3& _translation, const bool& _onGround = false);
    void rotate(const float& _angle, const vec3& _axis, const bool& _verticalAxis = false);
    void rotate(const mat4& _rotation, const bool& _verticalAxis = false);
    void yaw(const float& _yaw);
    void pitch(const float& _pitch);
    void roll(const float& _roll);

//    //SETTERS
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

    void setSize(QSize _size);
    void setSize(int _width, int _height); 
    void setWidth(int _width);
    void setHeight(int _height);
    void setBackgroundColor(QColor _backgroundColor);
    void setBackgroundColor(float red, float green, float blue, float alpha = 1.0f);
    void setViewCenter(vec3 _viewCenter);
    void setNearPlane(float _nearPlane);
    void setFarPlane(float _farPlane);
    void setAu(double _au);
    void setAv(double _av);
    void setKu(double _ku);
    void setKv(double _kv);
    void setU0(double _u0);
    void setV0(double _v0);
    void setNearPlane(double _nearPlane);
    void setFarPlane(double _farPlane);
    void setFOV(float _fov);
    void setFOV(double _fov);
    void setProjectionType(Camera::ProjectionType _projectionType);
    void setCustomProjection(mat4 _customProjection);
    void setViewPoint(Camera::ViewPoint _viewPoint);
    void setcMw(mat4 _cMw);
    void setwMc(mat4 _wMc);
    void lookAt(vec3 _point);
    void lookAt(Model3D* model);
    void setView(Model3D* model, Camera::View view);

signals:
    void sizeChanged();
    void fovChanged(float);

private:
//    float viewCenterDistance;
    vec3 viewCenter;

    QSize size;
    QOpenGLFramebufferObject* FBO;
    QOpenGLFramebufferObject* textureFBO;
    QColor backgroundColor;
    unsigned int samples;
    ProjectionType projectionType;
    mat4 customProjection;
    ViewPoint viewPoint;

    float nearPlane, farPlane;
    float au, av;
    float ku, kv;
    float u0, v0;

    mat4 proj;
    mat4 view;
};

#endif // CAMERA_H
