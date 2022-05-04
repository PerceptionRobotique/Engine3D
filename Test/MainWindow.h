#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QInputDialog>
#include <QGraphicsView>
#include <QPixmap>
#include <QImage>
#include <QKeyEvent>
#include <QListWidgetItem>
#include <QSettings>
#include <QEventLoop>

#include <QDebug>

#include <glm/gtx/string_cast.hpp>

#include <Engine3D.h>
#include <CameraController.h>
#include <Model3D.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

using namespace MIS;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionOpenFile_triggered();
    void on_modelsListWidget_itemDoubleClicked(QListWidgetItem* item);
    void updateModelLoading(Model3D* model, unsigned int progressValue);

    void updateVertexOnRAM();
    void updateVertexOnVRAM();

private:
    Ui::MainWindow* ui;
    QSettings settings;

    void keyPressEvent(QKeyEvent* e) override;
};

#endif // MAINWINDOW_H
