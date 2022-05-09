#include <iostream>

#include <QGuiApplication>
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

    //QOpenGLContext context;
    //QOffscreenSurface surface;
    //surface.create();
    //context.create();
    //context.makeCurrent(&surface);

    Engine3D engine(Engine3D::THREADED);

    QEventLoop loop;
    QObject::connect(&engine, SIGNAL(initializationFinished()), &loop, SLOT(quit()));
    engine.initialize();
    if(engine.getRenderMode() == Engine3D::THREADED) loop.exec();

    //engine.setFrameCounterEnabled(true);
    engine.setViewDistanceEnabled(false);
    engine.setMaxVertexLimitEnabled(false);
    engine.getMainCamera()->setSamples(8);

    engine.openModel(diskRoot + "/Users/nvill/3D Objects/ECathedrale/Other_Models/IntTout50/IntTout50.oct");
    
    //engine.getMainCamera()->translate(vec3(0, 5, 0));
    //engine.update();
    //engine.getFrame().save("frame1.png");
    //engine.setWaitLoading(true);
    //engine.getMainCamera()->translate(vec3(0, 5, 0));
    //engine.update();
    //engine.getFrame().save("frame2.png");
    //engine.setWaitLoading(false);
    //engine.getMainCamera()->translate(vec3(0, 5, 0));
    //engine.update();
    //engine.getFrame().save("frame3.png");
    //engine.setWaitLoading(true);
    //engine.getMainCamera()->translate(vec3(0, 5, 0));
    //engine.update();
    //engine.getFrame().save("frame4.png");

    //engine.getMainCamera()->setSize(QSize(1920, 1200));
    TrajectoryManager tm;
    tm.setFPS(25);
    //tm.setFPS(120);
    tm.addPose(*engine.getMainCamera());
    engine.getMainCamera()->translate(vec3(0, 15, 0));
    tm.addPose(*engine.getMainCamera());
    engine.getMainCamera()->rotate(180, vec3(0, 1, 0));
    tm.addPose(*engine.getMainCamera(), 3000);

    engine.update();
    engine.update();
    engine.update();
    engine.setWaitLoading(true);

    QDir().mkdir("video");
    for (unsigned int frameNumber = 0; frameNumber < tm.getTotalFrameNumber(); frameNumber++)
    {
        engine.getMainCamera()->setPose(tm.getFrame(frameNumber).pose);
        engine.takePicture().save("video/" + QString::number(frameNumber) + ".png");
        qDebug() << frameNumber + 1 << "/" << tm.getTotalFrameNumber();
    }

    VideoWriter videoWriter;
    videoWriter.setFramesPath("video");
    videoWriter.setFPS(25);
    videoWriter.setVCodec("hevc");
    videoWriter.setVideoFileName("video");
    videoWriter.writeVideo();
    videoWriter.waitForFinished();
    //while (!videoWriter.waitForFinished(0))
    //{
    //    QByteArray output = videoWriter.readAllStandardOutput();
    //    QByteArray error = videoWriter.readAllStandardError();
    //    if (!output.isEmpty()) std::cout << output.constData();
    //    if (!error.isEmpty()) std::cout << error.constData();
    //}

    //context.doneCurrent();
    engine.destroy();
    return 0;
}
