#ifndef MODEL3D_H
#define MODEL3D_H

#include <QMutex>
#include <QVector>
#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QFile>
#include <QFileInfo>
#include <QColor>

#include <glm/common.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Object3DQt.h"

using namespace glm;

class ENGINE3D_EXPORT Model3D : public Object3DQt
{
    Q_OBJECT

public:
    enum Primitives{
        TRIANGLES = GL_TRIANGLES,
        POINTS = GL_POINTS,
        LINES = GL_LINES
    };

    struct AABB{
        vec3 center = vec3(0, 0, 0);
        vec3 gravity = vec3(0, 0, 0);
        vec3 min = vec3(0, 0, 0);
        vec3 max = vec3(0, 0, 0);
    };

    struct StoredPose{
        vec3 position = vec3(0, 0, 0);
        vec3 rotation = vec3(0, 0, 0);

        StoredPose(mat4 m = mat4(1.0))
        {
            setMatrix(m);
        };

        StoredPose(const StoredPose& p)
        {
            position = p.position;
            rotation = p.rotation;
        };

        void setMatrix(mat4 m) {
            position = m[3];
            extractEulerAngleXYZ(m, rotation.x, rotation.y, rotation.z);
        };

        mat4 getMatrix() {
            mat4 m(1.0);
            m = glm::rotate(m, rotation.x, vec3(1.0f, 0.0f, 0.0f));
            m = glm::rotate(m, rotation.y, vec3(0.0f, 1.0f, 0.0f));
            m = glm::rotate(m, rotation.z, vec3(0.0f, 0.0f, 1.0f));
            m[3] = vec4(position, 1.0f);
            return m;
        };
    };

    Model3D(QString _fileName = "");
    ~Model3D();

    static unsigned int model3DNumber;
    static QOpenGLBuffer boxIndexBuffer;
    static QVector<unsigned int> boxIndex;
    static unsigned long long getVertexOnRAM();
    static unsigned long long getVertexOnVRAM();

    QString getName() const;

    Primitives getPrimitives() const;
    mat4 getwMo() const;
    AABB getAABB() const;
    QVector<glm::vec3> getBox();
    bool hasIntensity() const;
    bool getShowIntensity() const;
    unsigned long long getVertexNumber() const;
    bool isPrepared() const;
    bool isOnRAM() const;
    bool isOnVRAM() const;

public slots:
    virtual void loadRAM() = 0;
    void unloadRAM();
    void loadVRAM();
    void unloadVRAM();

    void loadBoxRAM();
    void unloadBoxRAM();
    void loadBoxVRAM();
    void unloadBoxVRAM();

    void draw(QOpenGLShaderProgram* shader);

private:
    static QMutex vertexMutex;
    static unsigned long long vertexOnRAM;
    static unsigned long long vertexOnVRAM;

    bool showIntensity;
    bool onVRAM;

protected:
    static void setVertexOnRAM(unsigned long long value);
    static void setVertexOnVRAM(unsigned long long value);

    QString fileName;
    QFile file;
    QFileInfo fileInfo;
    QString name;

    bool prepared;
    bool m_hasIntensity;

    QMutex ramMutex;
    bool onRAM;

    Primitives primitives;
    unsigned long long vertexNumber;
    AABB aabb;
    QVector<glm::vec3> box;
    QVector<glm::vec3> pos;
    QVector<glm::vec3> color;
    QVector<float> intensity;

    bool globalColorEnabled;
    QColor globalColor;

    QOpenGLBuffer boxBuffer;
    QOpenGLBuffer posBuffer;
    QOpenGLBuffer colorBuffer;
    QOpenGLBuffer intensityBuffer;

signals:
    void modelChanged();
};

inline Model3D::AABB operator*(const mat4& matrix, const Model3D::AABB& aabb)
{
    Model3D::AABB _aabb;
    _aabb.min = vec3(vec4(matrix * vec4(aabb.min, 1.0f))/vec4(matrix * vec4(aabb.min, 1.0f)).w);
    _aabb.max = vec3(vec4(matrix * vec4(aabb.max, 1.0f))/vec4(matrix * vec4(aabb.max, 1.0f)).w);
    _aabb.center = vec3(vec4(matrix * vec4(aabb.center, 1.0f))/vec4(matrix * vec4(aabb.center, 1.0f)).w);
    _aabb.gravity = vec3(vec4(matrix * vec4(aabb.gravity, 1.0f))/vec4(matrix * vec4(aabb.gravity, 1.0f)).w);
    return _aabb;
}

inline Model3D::AABB operator*(const Model3D::AABB& aabb, const mat4& matrix)
{
    Model3D::AABB _aabb;
    _aabb.min = vec3(vec4(vec4(aabb.min, 1.0f) * matrix)/vec4(vec4(aabb.min, 1.0f) * matrix).w);
    _aabb.max = vec3(vec4(vec4(aabb.max, 1.0f) * matrix)/vec4(vec4(aabb.max, 1.0f) * matrix).w);
    _aabb.center = vec3(vec4(vec4(aabb.center, 1.0f) * matrix)/vec4(vec4(aabb.center, 1.0f) * matrix).w);
    _aabb.gravity = vec3(vec4(vec4(aabb.gravity, 1.0f) * matrix)/vec4(vec4(aabb.gravity, 1.0f) * matrix).w);
    return _aabb;
}

#endif // MODEL3D_H
