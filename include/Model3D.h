#ifndef MODEL3D_H
#define MODEL3D_H

#include <QMutex>
#include <QThread>
#include <QVector>
#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QFile>
#include <QFileInfo>
#include <QColor>
#include <QSettings>
#include <QVariant>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>

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
    enum Primitives : GLuint {
        POINTS = GL_POINTS,
        LINES = GL_LINES,
        TRIANGLES = GL_TRIANGLES
    };

    struct AABB{
        vec3 min = vec3(0, 0, 0);
        vec3 max = vec3(0, 0, 0);
        vec3 center = vec3(0, 0, 0);
        vec3 gravity = vec3(0, 0, 0);

        void updateCenter()
        {
            center.x = (max.x + min.x) / 2.0f;
            center.y = (max.y + min.y) / 2.0f;
            center.z = (max.z + min.z) / 2.0f;
        };

        bool pointInBox(glm::vec3 point)
        {
            return point.x >= min.x
                && point.y >= min.y
                && point.z >= min.z
                
                && point.x <= max.x
                && point.y <= max.y
                && point.z <= max.z;
        }
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

    Model3D(QString _fileName);
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
    QVector<glm::vec3> getBox() const;
    bool getShowIntensity() const;
    unsigned long long getVertexNumber() const;
    QVector<glm::vec3>& getPos();
    QVector<unsigned char>& getColor();
    QVector<unsigned char>& getIntensity();

    bool hasIntensity() const;
    bool isPrepared() const;
    bool isOnScreen() const;
    bool isOnRAM() const;
    bool isOnVRAM() const;
    void waitRAMloading();

public slots:
    //visibility
    void setVisible(bool _visible);
    void setBoxVisible(bool _boxVisible);

    //on screen
    void setOnScreen(bool _onScreen, bool force = false);

    //AABB
    void setAABB(AABB _aabb);

    //RAM
    void loadRAM(bool force = false);
    void unloadRAM(bool force = false);

    //VRAM
    void loadVRAM();
    void unloadVRAM();

    //Box RAM
    void loadBoxRAM();
    void unloadBoxRAM();

    //Box VRAM
    void loadBoxVRAM();
    void unloadBoxVRAM();

    virtual bool draw(QOpenGLShaderProgram* shader);
    virtual bool drawBox(QOpenGLShaderProgram* shader);

    void setwMo(mat4 wMo);

private:
    static QMutex vertexNumberMutex;
    static unsigned long long vertexOnRAM;
    static unsigned long long vertexOnVRAM;
    void setVertexOnVRAM(unsigned long long value);

    QString fileName;
    QString name;

    bool showIntensity;
    bool visible;
    bool boxVisible;
    bool onScreen;
    QMutex boxLoaderMutex;
    bool boxOnRAM;
    bool boxOnVRAM;
    bool onVRAM;

    bool globalColorEnabled;
    QColor globalColor;
    QColor boxColor;

    AABB aabb;
    QVector<glm::vec3> box;
    QOpenGLBuffer boxBuffer;
    QOpenGLBuffer posBuffer;
    QOpenGLBuffer colorBuffer;
    QOpenGLBuffer intensityBuffer;

protected:
    void setVertexOnRAM(unsigned long long value);

    QFile* file;
    QFileInfo fileInfo;
    QSettings* settings;

    Primitives primitives;
    bool prepared;
    bool liveLoading;
    bool m_hasIntensity;

    QMutex vertexLoader; //avoids load and unload at the same time
    QFuture<void> ramLoader;
    bool onRAM;
    virtual void loadRAMthread() = 0;
    void endRAMloading();

    unsigned long long vertexNumber;
    QVector<glm::vec3> pos;
    QVector<unsigned char> color;
    QVector<unsigned char> intensity;

signals:
    void modelCreated();
    void modelChanged();
    void modelLoadingDelayed();
    void modelLoadingUpdate(Model3D* model, unsigned int value);
    void modelLoaded();
    void vertexOnRAMChanged(unsigned long long vertexOnRAM);
    void vertexOnVRAMChanged(unsigned long long vertexOnVRAM);
    void modelDestroyed();
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

inline QDebug operator<<(QDebug os, const Model3D::AABB& aabb)
{
    os << "AABB:{" << Qt::endl;
    os << "min(" + QString::fromStdString(glm::to_string(aabb.min)) + ")" << Qt::endl;
    os << "max(" + QString::fromStdString(glm::to_string(aabb.max)) + ")" << Qt::endl;
    os << "center(" + QString::fromStdString(glm::to_string(aabb.center)) + ")" << Qt::endl;
    os << "gravity(" + QString::fromStdString(glm::to_string(aabb.gravity)) + ")" << Qt::endl;
    os << "}";
    return os;
}

#endif // MODEL3D_H
