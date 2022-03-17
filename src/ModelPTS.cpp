#include "ModelPTS.h"

ModelPTS::ModelPTS(QString _fileName)
    : Model3D(_fileName)
{
    primitives = Model3D::POINTS;

    QTextStream ts(&file);
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

    QThread* ramLoader = QThread::create(&ModelPTS::loadRAM, this);
    connect(ramLoader, SIGNAL(finished()), ramLoader, SLOT(deleteLater()));
    ramLoader->start();
}

void ModelPTS::loadRAM()
{
    static QMutex ramLoaderMutex;

    if (ramLoaderMutex.try_lock())
    {
        if (!onRAM)
        {
            int loadingPourcentage = 0;
            bool firstBlocComputed = false;
            QTextStream ts(&file);

            QString halfLine = "";
            QString line = ts.readLine();
            QStringList elements = line.split(' ');
            elements.removeAll("");
            if (elements.count() >= 3)
            {
                ts.seek(0);
                ts.flush();
            }

            QVector<QThread*> loaders;
            QList<QStringList> computedLines;
            QList<AABB*> computedAABB;
            QList<QVector<glm::vec3>*> computedPos;
            QList<QVector<glm::vec3>*> computedColor;
            QList<QVector<float>*> computedIntensity;

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
                computedColor.append(new QVector<glm::vec3>);
                computedIntensity.append(new QVector<float>);

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
                computedColor.append(new QVector<glm::vec3>);
                computedIntensity.append(new QVector<float>);

                loaders.append(QThread::create(&ModelPTS::computePTSLines, this, computedLines.last(), computedAABB.last(), computedPos.last(), computedColor.last(), computedIntensity.last()));
                loaders.last()->start();

                if ((int)(pos.count() / vertexNumber * 100.0f) > loadingPourcentage)
                {
                    loadingPourcentage = pos.count() / vertexNumber * 100.0f;
                    //emit modelLoadingUpdate(this, loadingPourcentage);
                }
            }

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

                if ((int)(pos.count() / vertexNumber * 100.0f) > loadingPourcentage)
                {
                    loadingPourcentage = pos.count() / vertexNumber * 100.0f;
                    //emit modelLoadingUpdate(this, loadingPourcentage);
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
            for (QVector<glm::vec3>* _color : computedColor) delete _color;
            for (QVector<float>* _intensity : computedIntensity) delete _intensity;

            aabb.gravity /= vertexNumber;
            aabb.center = (aabb.max + aabb.min) / 2.0f;

            prepared = true;
            onRAM = true;
            Model3D::setVertexOnRAM(Model3D::getVertexOnRAM() + vertexNumber);
        }

        ramLoaderMutex.unlock();
    }
}

void ModelPTS::computePTSLines(const QStringList& lines, Model3D::AABB* currentAABB, QVector<glm::vec3>* currentPos, QVector<glm::vec3>* currentColor, QVector<float>* currentIntensity)
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
                    currentIntensity->append((elements.takeFirst().toInt() + 2048.0f) / 4096.0f);
            }

            glm::vec3 color;
            for (unsigned int i = 0; i < 3; i++) color[i] = ((float)elements.takeFirst().toUShort()) / 255.0f;
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
