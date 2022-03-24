#include "ModelPTS.h"

ModelPTS::ModelPTS(QString _fileName)
    : Model3D(_fileName)
{
    liveLoading = false;
    primitives = Model3D::POINTS;

    QTextStream ts(file);
    QString text;
    QStringList elements;

    text = ts.readLine();
    elements = text.split(' ');
    elements.removeAll("");

    if (elements.count() == 1)
    {
        vertexNumber = elements.first().toULongLong();
        text = ts.readLine();
        elements = text.split(' ');
    }
    else
    {
        ts.seek(0);
        ts.flush();
        vertexNumber = 1;
        while (!ts.atEnd())
        {
            text = ts.read(BYTES_PER_READ);
            vertexNumber += text.count('\n');
        }
        elements = text.split(' ');
        vertexNumber -= elements.last().count('\n');

        ts.seek(0);
        ts.flush();
        text = ts.readLine();
        elements = text.split(' ');
        elements.removeAll("");
    }

    if (elements.count() > 6)
        m_hasIntensity = true;

    ts.seek(0);
    ts.flush();

    vertexLoader.lock();
    ramLoader = QThread::create(&ModelPTS::loadRAMthread, this);
    connect(ramLoader, SIGNAL(finished()), this, SLOT(loadingRAMfinished()));
    ramLoader->start();
}

ModelPTS::~ModelPTS()
{
    if (ramLoader != nullptr)
    {
        ramLoader->requestInterruption();
        ramLoader->wait();
        delete ramLoader;
        ramLoader = nullptr;
    }
}

void ModelPTS::loadRAMthread()
{
    int loadingPourcentage = 0;
    bool firstBlocComputed = false;
    QTextStream ts(file);

    QString halfLine = "";
    QString line = ts.readLine();
    QStringList elements = line.split(' ');
    elements.removeAll("");
    if (elements.count() >= 3)
    {
        ts.seek(0);
        ts.flush();
    }

    AABB aabb;
    QVector<QThread*> loaders;
    QList<QStringList> computedLines;
    QList<AABB*> computedAABB;
    QList<QVector<glm::vec3>*> computedPos;
    QList<QVector<unsigned char>*> computedColor;
    QList<QVector<unsigned char>*> computedIntensity;

    for (unsigned int i = 0; i < (unsigned int)QThread::idealThreadCount() && !ts.atEnd() && !QThread::currentThread()->isInterruptionRequested(); i++)
    {
        QString text = halfLine + ts.read(BYTES_PER_READ);
        computedLines.append(text.split('\n'));
        if (!ts.atEnd())
        {
            halfLine = computedLines.last().last();
            computedLines.last().pop_back();
        }

        computedAABB.append(new AABB);
        computedPos.append(new QVector<glm::vec3>);
        computedColor.append(new QVector<unsigned char>);
        computedIntensity.append(new QVector<unsigned char>);

        loaders.append(QThread::create(&ModelPTS::computePTSLines, this, computedLines.last(), computedAABB.last(), computedPos.last(), computedColor.last(), computedIntensity.last()));
        loaders.last()->start();
    }

    while (!ts.atEnd() && !QThread::currentThread()->isInterruptionRequested())
    {
        loaders.first()->wait();

        //AABB
        if (!firstBlocComputed)
        {
            aabb = *computedAABB.first();
            firstBlocComputed = true;
        }
        else
        {
            if (computedAABB.first()->min.x < aabb.min.x) aabb.min.x = computedAABB.first()->min.x;
            if (computedAABB.first()->min.y < aabb.min.y) aabb.min.y = computedAABB.first()->min.y;
            if (computedAABB.first()->min.z < aabb.min.z) aabb.min.z = computedAABB.first()->min.z;

            if (computedAABB.first()->max.x > aabb.max.x) aabb.max.x = computedAABB.first()->max.x;
            if (computedAABB.first()->max.y > aabb.max.y) aabb.max.y = computedAABB.first()->max.y;
            if (computedAABB.first()->max.z > aabb.max.z) aabb.max.z = computedAABB.first()->max.z;
        }

        //BUFFERS
        pos.append(*computedPos.first());
        color.append(*computedColor.first());
        intensity.append(*computedIntensity.first());

        loaders.first()->deleteLater();
        delete computedAABB.first();
        delete computedPos.first();
        delete computedColor.first();
        delete computedIntensity.first();
        ;
        loaders.pop_front();
        computedLines.pop_front();
        computedAABB.pop_front();
        computedPos.pop_front();
        computedColor.pop_front();
        computedIntensity.pop_front();

        QString text = halfLine + ts.read(BYTES_PER_READ);
        computedLines.append(text.split('\n'));
        if (!ts.atEnd())
        {
            halfLine = computedLines.last().last();
            computedLines.last().pop_back();
        }

        computedAABB.append(new AABB);
        computedPos.append(new QVector<glm::vec3>);
        computedColor.append(new QVector<unsigned char>);
        computedIntensity.append(new QVector<unsigned char>);

        loaders.append(QThread::create(&ModelPTS::computePTSLines, this, computedLines.last(), computedAABB.last(), computedPos.last(), computedColor.last(), computedIntensity.last()));
        loaders.last()->start();

        if ((int)(100.0f * pos.count() / vertexNumber) > loadingPourcentage)
        {
            loadingPourcentage = 100.0f * pos.count() / vertexNumber;
            emit modelLoadingUpdate(this, loadingPourcentage);
        }
    }
    ts.seek(0);
    ts.flush();

    while (!loaders.isEmpty() && !QThread::currentThread()->isInterruptionRequested())
    {
        loaders.first()->wait();
        loaders.first()->deleteLater();

        //AABB
        if (!firstBlocComputed)
        {
            aabb = *computedAABB.first();
            firstBlocComputed = true;
        }
        else
        {
            if (computedAABB.first()->min.x < aabb.min.x) aabb.min.x = computedAABB.first()->min.x;
            if (computedAABB.first()->min.y < aabb.min.y) aabb.min.y = computedAABB.first()->min.y;
            if (computedAABB.first()->min.z < aabb.min.z) aabb.min.z = computedAABB.first()->min.z;

            if (computedAABB.first()->max.x > aabb.max.x) aabb.max.x = computedAABB.first()->max.x;
            if (computedAABB.first()->max.y > aabb.max.y) aabb.max.y = computedAABB.first()->max.y;
            if (computedAABB.first()->max.z > aabb.max.z) aabb.max.z = computedAABB.first()->max.z;
        }

        //BUFFERS
        pos.append(*computedPos.first());
        color.append(*computedColor.first());
        intensity.append(*computedIntensity.first());

        loaders.first()->deleteLater();
        delete computedAABB.first();
        delete computedPos.first();
        delete computedColor.first();
        delete computedIntensity.first();
        ;
        loaders.pop_front();
        computedLines.pop_front();
        computedAABB.pop_front();
        computedPos.pop_front();
        computedColor.pop_front();
        computedIntensity.pop_front();

        if ((int)(100.0f * pos.count() / vertexNumber) > loadingPourcentage)
        {
            loadingPourcentage = 100.0f * pos.count() / vertexNumber;
            emit modelLoadingUpdate(this, loadingPourcentage);
        }
    }

    if (QThread::currentThread()->isInterruptionRequested())
    {
        for (QThread* loader : loaders) loader->requestInterruption();
        for (QThread* loader : loaders)
        {
            loader->wait();
            delete loader;
        }
        loaders.clear();
    }

    for (AABB* _aabb : computedAABB) delete _aabb;
    for (QVector<glm::vec3>* _pos : computedPos) delete _pos;
    for (QVector<unsigned char>* _color : computedColor) delete _color;
    for (QVector<unsigned char>* _intensity : computedIntensity) delete _intensity;

    aabb.gravity /= vertexNumber;
    aabb.center = (aabb.max + aabb.min) / 2.0f;
    setAABB(aabb);

    prepared = true;
}

void ModelPTS::computePTSLines(const QStringList& lines, Model3D::AABB* currentAABB, QVector<glm::vec3>* currentPos, QVector<unsigned char>* currentColor, QVector<unsigned char>* currentIntensity)
{
    bool firstLinePassed = false;
    for (const QString& line : lines)
    {
        QStringList elements = line.split(' ');
        if (elements.count() >= 6)
        {
            vec3 point;
            for (unsigned int i = 0; i < 3; i++) point[i] = elements.takeFirst().toFloat();
            currentPos->append(point);

            if (m_hasIntensity)
            {
                if (elements.count() <= 3)
                    currentIntensity->append(0);
                else
                    currentIntensity->append((unsigned char)((elements.takeFirst().toInt() + 2048.0) / 4096.0 * 255.0));
            }

            QVector<unsigned char> color(3);
            for (unsigned int i = 0; i < 3; i++) color[i] = elements.takeFirst().toUShort();
            currentColor->append(color);

            //AABB
            currentAABB->gravity += point;
            if (!firstLinePassed)
            {
                currentAABB->min = point;
                currentAABB->max = point;
                firstLinePassed = true;
            }
            else
            {
                if (point.x < currentAABB->min.x) currentAABB->min.x = point.x;
                if (point.y < currentAABB->min.y) currentAABB->min.y = point.y;
                if (point.z < currentAABB->min.z) currentAABB->min.z = point.z;

                if (point.x > currentAABB->max.x) currentAABB->max.x = point.x;
                if (point.y > currentAABB->max.y) currentAABB->max.y = point.y;
                if (point.z > currentAABB->max.z) currentAABB->max.z = point.z;
            }
        }
        if (QThread::currentThread()->isInterruptionRequested())
            break;
    }
}
