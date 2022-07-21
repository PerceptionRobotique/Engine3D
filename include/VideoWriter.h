#ifndef VIDEOWRITERR_H
#define VIDEOWRITERR_H

#include "Engine3D_global.h"

#include <QProcess>
#include <QMutex>
#include <QFile>
#include <QDir>

namespace MIS
{

	/**
	 * @brief      This class describes a video writer.
	 */
	class ENGINE3D_EXPORT VideoWriter : public QProcess
	{
		Q_OBJECT

	public:
		VideoWriter(QObject* parent = nullptr);
		~VideoWriter();

	public slots:
		void setVideoFileName(const QString& _videoFileName);
		void setVideoSuffix(QString _videoSuffix);
		void setFramesPath(const QString& _framesPath);
		void setFramesPrefix(const QString& _framesPrefix);
		void setFramesSuffix(const QString& _framesSuffix);
		void setVCodec(const QString& _vcodec);
		void setFPS(unsigned int _fps);
		void setCRF(unsigned int _crf);
		void setPixelFormat(const QString& _pixelFormat);

		void writeVideo();

	private:
#ifdef _WIN32
		static unsigned int instanceNumber;
		static QMutex instanceNumberMutex;
#endif
		static QString ffmpeg;

		QString videoFileName;
		QString videoSuffix;
		QString framesPath;
		QString framesPrefix;
		QString framesSuffix;
		QString vcodec;
		QString pixelFormat;
		unsigned int fps;
		int crf;
	};

}

#endif // VIDEOWRITERR_H