#ifndef MODEL3D_H
#define MODEL3D_H

#include <QMutex>
#include <QVector>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QFile>
#include <QFileInfo>
#include <QColor>
#include <QSettings>
#include <QVariant>
#include <QtConcurrent/QtConcurrent>

#include <glm/common.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Object3DQt.h"

using namespace glm;

namespace MIS
{

    class ENGINE3D_EXPORT Model3D : public Object3DQt
    {
        Q_OBJECT

    public:
        enum Primitives : GLuint {
            POINTS = GL_POINTS,
            LINES = GL_LINES,
            TRIANGLES = GL_TRIANGLES
        };

        struct AABB {
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

            QVector<float> toVector()
            {
                QVector<float> vector;
                vector.append(min.x);
                vector.append(min.y);
                vector.append(min.z);
                vector.append(max.x);
                vector.append(max.y);
                vector.append(max.z);
                return vector;
            }

            void fix()
            {
                for (unsigned int i = 0; i < 3; i++)
                {
                    if (min[i] > max[i])
                    {
                        float t = min[i];
                        min[i] = max[i];
                        max[i] = t;
                    }
                }
            }
        };

        Model3D(QString _fileName, QOpenGLShaderProgram* shader = nullptr, QOpenGLShaderProgram* boxShader = nullptr);
        ~Model3D();

        static unsigned int model3DNumber;
        static QOpenGLBuffer boxIndexBuffer;
        static QVector<unsigned int> boxIndex;
        static unsigned long long getVertexOnRAM();
        static unsigned long long getVertexOnVRAM();

        QString getName() const;

        Primitives getPrimitives() const;
        virtual float getScale() const;
        virtual mat4 getwMo() const;
        AABB getAABB() const;
        bool isPointInAABB(vec3 point) const;
        QVector<glm::vec3> getBox() const;
        unsigned long long getVertexNumber() const;
        QVector<glm::vec3>& getPos();
        QVector<unsigned char>& getColor();
        QVector<unsigned char>& getIntensity();
        virtual glm::vec3 getPosAt(unsigned long long index);
        virtual QVector<unsigned char> getColorAt(unsigned long long index);
        virtual unsigned char getIntensityAt(unsigned long long index);

        virtual bool hasIntensity() const;
        virtual bool getShowIntensity() const;
        virtual bool isVisible() const;
        virtual bool isBoxVisible() const;
        bool isPrepared() const;
        bool isLiveLoading() const;
        bool isOnRAM() const;
        bool isOnVRAM() const;

        virtual bool isGlobalColorEnabled() const;
        virtual QColor getGlobalColor() const;

        //Poses
        QHash<QString, mat4> getStoredPoses() const;
        mat4 getStoredPose(const QString& name) const;

    public slots:
        //Visibility
        void setVisible(bool _visible);
        void setBoxVisible(bool _boxVisible);
        void setShowIntensity(bool _showIntensity);

        //AABB
        void setAABB(AABB _aabb);

        //Shaders
        void setShader(QOpenGLShaderProgram* shader);
        void setBoxShader(QOpenGLShaderProgram* boxShader);

        //RAM
        void loadRAM(bool force = false);
        void unloadRAM(bool force = false);

        //VRAM
        void loadVRAM(bool force = false);
        void unloadVRAM(bool force = false);

        //Box RAM
        void loadBoxRAM();
        void unloadBoxRAM();

        //Box VRAM
        void loadBoxVRAM();
        void unloadBoxVRAM();

        //Draw
        virtual bool draw();
        virtual bool drawBox();

        void setScale(double _scale);
        void setwMo(mat4 wMo);
        void setGlobalColorEnabled(bool enabled);
        void setGlobalColor(QColor color);

        //Poses
        bool addStoredPose(const QString &name, mat4 pose);
        void removeStoredPose(const QString& name);

    private:
        static QMutex vertexNumberMutex;
        static unsigned long long vertexOnRAM;
        static unsigned long long vertexOnVRAM;
        static QVector<unsigned long long> vertexAddedToRAM;
        static QVector<unsigned long long> vertexAddedToVRAM;
        void addVertexOnVRAM();
        void removeVertexOnVRAM();

        QString fileName;
        QString name;

        bool showIntensity;
        bool visible;
        bool boxVisible;
        QMutex boxLoaderMutex;
        bool boxOnRAM;
        bool boxOnVRAM;
        bool onVRAM;

        bool globalColorEnabled;
        QColor globalColor;
        QColor boxColor;

        AABB aabb;
        QVector<glm::vec3> box;
        QOpenGLShaderProgram* boxShader;

        QHash<QString, mat4> storedPoses;

    protected:
        void addVertexOnRAM();
        void removeVertexOnRAM();

        QFile* file;
        QFileInfo fileInfo;
        QSettings* settings;

        Primitives primitives;
        bool prepared;
        bool liveLoading;
        bool m_hasIntensity;

        QMutex vertexLoader; //avoids load and unload at the same time
        bool onRAM;
        virtual void loadRamThread() = 0;
        void unloadRAMthread();
        virtual void render(QOpenGLFunctions* f) = 0;

        unsigned long long vertexNumber;
        float scale;
        QVector<glm::vec3> pos;
        QVector<unsigned char> color;
        QVector<unsigned char> intensity;

        QOpenGLShaderProgram* shader;
        QOpenGLBuffer boxBuffer;
        QOpenGLBuffer posBuffer;
        QOpenGLBuffer colorBuffer;
        QOpenGLBuffer intensityBuffer;

        QVector<QOpenGLBuffer> pointBuffer;
        QVector<QOpenGLBuffer> normalBuffer;
        QVector<QOpenGLBuffer> uvBuffer;
        QHash<QString, QOpenGLTexture*> texturesBuffers;

        // OBJ //
        QVector<QVector<glm::vec3>> point;
        QVector<QVector<glm::vec2>> uv;
        QVector<QVector<glm::vec3>> normal;
        QHash<QString, QImage> textures;

    signals:
        void modelCreated();
        void modelChanged();
        void modelRenderChanged();
        void modelMoved();
        void modelLoadingDelayed();
        void modelLoadingUpdate(Model3D* model, unsigned int value);
        void modelLoaded();
        void modelUnloaded();
        void vertexOnRAMChanged();
        void vertexOnVRAMChanged();
        void modelDestroyed();
    };

    inline Model3D::AABB operator*(const mat4& matrix, const Model3D::AABB& aabb)
    {
        Model3D::AABB _aabb;
        _aabb.min = vec3(vec4(matrix * vec4(aabb.min, 1.0f)) / vec4(matrix * vec4(aabb.min, 1.0f)).w);
        _aabb.max = vec3(vec4(matrix * vec4(aabb.max, 1.0f)) / vec4(matrix * vec4(aabb.max, 1.0f)).w);
        _aabb.center = vec3(vec4(matrix * vec4(aabb.center, 1.0f)) / vec4(matrix * vec4(aabb.center, 1.0f)).w);
        _aabb.gravity = vec3(vec4(matrix * vec4(aabb.gravity, 1.0f)) / vec4(matrix * vec4(aabb.gravity, 1.0f)).w);
        _aabb.fix();
        return _aabb;
    }

    inline Model3D::AABB operator*(const Model3D::AABB& aabb, const mat4& matrix)
    {
        Model3D::AABB _aabb;
        _aabb.min = vec3(vec4(vec4(aabb.min, 1.0f) * matrix) / vec4(vec4(aabb.min, 1.0f) * matrix).w);
        _aabb.max = vec3(vec4(vec4(aabb.max, 1.0f) * matrix) / vec4(vec4(aabb.max, 1.0f) * matrix).w);
        _aabb.center = vec3(vec4(vec4(aabb.center, 1.0f) * matrix) / vec4(vec4(aabb.center, 1.0f) * matrix).w);
        _aabb.gravity = vec3(vec4(vec4(aabb.gravity, 1.0f) * matrix) / vec4(vec4(aabb.gravity, 1.0f) * matrix).w);
        _aabb.fix();
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
}

#endif // MODEL3D_H
