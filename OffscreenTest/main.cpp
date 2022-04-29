#include <iostream>

#include <QOpenGLContext>
#include <QScreen>
#include <QOffscreenSurface>

#include <QProcess>

#include <Engine3D.h>
#include <TrajectoryManager.h>
#include <VideoWriter.h>

using namespace MIS;

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);

    QString diskRoot;
#ifdef _WIN32
    diskRoot = "C:";
#elif __linux__
    diskRoot = "/mnt/c";
#endif

    Engine3D* engine = new Engine3D;
    QOpenGLContext context;
    QOffscreenSurface surface;

    surface.create();
    context.create();
    context.makeCurrent(&surface);
    engine->initialize();

    //engine->setWaitLoading(true);
    //engine->setViewDistanceEnabled(false);
    //engine->setMaxVertexLimitEnabled(false);
    //engine->getMainCamera()->setSamples(8);
    //engine->openModel(diskRoot + "/Users/nvill/3D Objects/ECathedrale/Other_Models/IntTout50/IntTout50.oct");

    //engine->getMainCamera()->setSize(QSize(1920, 1200));
    //TrajectoryManager tm;
    ////tm.setFPS(5);
    //tm.setFPS(120);
    //tm.addPose(*engine->getMainCamera());
    //engine->getMainCamera()->translate(vec3(0, 15, 0));
    //tm.addPose(*engine->getMainCamera());
    //engine->getMainCamera()->rotate(180, vec3(0, 1, 0));
    //tm.addPose(*engine->getMainCamera(), 3000);

    //QDir().mkdir("video");
    //for (unsigned int frameNumber = 0; frameNumber < tm.getTotalFrameNumber(); frameNumber++)
    //{
    //    qDebug() << frameNumber + 1 << "/" << tm.getTotalFrameNumber();
    //    engine->getMainCamera()->setPose(tm.getFrame(frameNumber).pose);
    //    engine->render();
    //    engine->getMainCamera()->toImage().save("video/" + QString::number(frameNumber) + ".png");
    //}

    VideoWriter videoWriter;
    videoWriter.setFramesPath("video");
    videoWriter.setFPS(120);
    videoWriter.setVCodec("hevc");
    videoWriter.setVideoFileName("test-hevc");
    videoWriter.writeVideo();
    while (!videoWriter.waitForFinished(0))
    {
        QByteArray output = videoWriter.readAllStandardOutput();
        QByteArray error = videoWriter.readAllStandardError();
        if (!output.isEmpty()) std::cout << output.constData();
        if (!error.isEmpty()) std::cout << error.constData();
    }

//#ifdef _WIN32
//    if (QFile(":/FFmpeg/ffmpeg.exe").exists())
//    {
//        qDebug() << "FFmpeg detected";
//        QDir().mkdir("FFmpeg");
//        for (QString fileName : QDir(":/FFmpeg").entryList())
//        {
//            QFile file(":/FFmpeg/" + fileName);
//            file.copy("FFmpeg/" + fileName);
//        }
//#endif
//
//    QStringList arguments;
//    arguments << "-framerate" << QString::number(tm.getFPS());
//    arguments << "-y";
//    arguments << "-i" << QDir("video").absolutePath() + "/%d.png";
//    arguments << "-c:v" << "libx264";
//    arguments << "video.mp4";
//    QProcess process;
//#ifdef _WIN32
//    process.start("./FFmpeg/ffmpeg.exe", arguments);
//#elif __linux__
//    process.start("ffmpeg", arguments);
//#endif
//    bool ok = process.waitForFinished(10000);
//    if (!ok) qDebug() << "Video creation failed.";
//#ifdef _WIN32
//    }
//    else qDebug() << "FFmpeg not detected";
//    QDir("FFmpeg").removeRecursively();
//#endif

    delete engine;
    context.doneCurrent();
    return 0;
}
