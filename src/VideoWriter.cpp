#include "VideoWriter.h"

namespace MIS
{

#ifdef _WIN32
    unsigned int VideoWriter::instanceNumber(0);
    QMutex VideoWriter::instanceNumberMutex;
    QString VideoWriter::ffmpeg(QDir::tempPath() + "/ESILab/FFmpeg/ffmpeg.exe");
#elif __linux__
    QString VideoWriter::ffmpeg("ffmpeg");
#endif

    VideoWriter::VideoWriter(QObject* parent)
        : QProcess(parent)
        , videoFileName("video")
        , videoSuffix(".mp4")
        , framesPath("")
        , framesPrefix("")
        , framesSuffix(".png")
        , vcodec("libx264")
        , fps(25)
        , crf(-1)
    {
#ifdef _WIN32
        instanceNumberMutex.lock();
        if (instanceNumber == 0)
        {
            if (QFile(":/FFmpeg/ffmpeg.exe").exists())
            {
                QDir().mkdir(QDir::tempPath() + "/ESILab");
                QDir().mkdir(QDir::tempPath() + "/ESILab/FFmpeg");
                for (QString fileName : QDir(":/FFmpeg").entryList())
                {
                    QFile file(":/FFmpeg/" + fileName);
                    file.copy(QDir::tempPath() + "/ESILab/FFmpeg/" + fileName);
                }
            }
        }
        instanceNumber++;
        instanceNumberMutex.unlock();
#endif
    }

    VideoWriter::~VideoWriter()
    {
#ifdef _WIN32
        instanceNumberMutex.lock();
        instanceNumber--;
        if (instanceNumber == 0) QDir(QDir::tempPath() + "/ESILab").removeRecursively();
        instanceNumberMutex.unlock();
#endif
    }

    void VideoWriter::setVideoFileName(const QString& _videoFileName)
    {
        QStringList splitted = _videoFileName.split(".");
        if (splitted.count() > 1)
        {
            videoFileName = "";
            for (unsigned int i = 0; i < splitted.count() - 1; i++)
            {
                if (i > 0) videoFileName.append('.');
                videoFileName.append(splitted[i]);
            }
            setVideoSuffix("." + splitted.last());
        }
        else videoFileName = _videoFileName;
    }

    void VideoWriter::setVideoSuffix(QString _videoSuffix)
    {
        if (_videoSuffix[0] != '.') _videoSuffix.insert(0, '.');
        videoSuffix = _videoSuffix;
    }

    void VideoWriter::setFramesPath(const QString& _framesPath)
    {
        framesPath = _framesPath;
    }

    void VideoWriter::setFramesPrefix(const QString& _framesPrefix)
    {
        framesPrefix = _framesPrefix;
    }

    void VideoWriter::setFramesSuffix(const QString& _framesSuffix)
    {
        framesSuffix = _framesSuffix;
    }

    void VideoWriter::setVCodec(const QString& _vcodec)
    {
        vcodec = _vcodec;
    }

    void VideoWriter::setFPS(unsigned int _fps)
    {
        fps = _fps;
    }

    void VideoWriter::setCRF(unsigned int _crf)
    {
        crf = _crf;
    }

    void VideoWriter::writeVideo()
    {
        if (state() == QProcess::NotRunning)
        {
            QStringList arguments;
            arguments << "-y";
            arguments << "-framerate" << QString::number(fps);
            arguments << "-i" << framesPath + "/" + framesPrefix + "%d" + framesSuffix;
            arguments << "-c:v" << vcodec;
            if(crf != -1) arguments << "-crf" << QString::number(crf);
            arguments << videoFileName + videoSuffix;

            start(ffmpeg, arguments);
        }
    }
}