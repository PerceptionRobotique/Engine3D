#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QInputDialog>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QImage>
#include <QKeyEvent>
#include <QListWidgetItem>
#include <QSettings>
#include <QEventLoop>

#include <QDebug>

#include <glm/gtx/string_cast.hpp>

#include <Model3D.h>
#include <Model3DWriter.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionOpenFile_triggered();
    void on_actionSaveFile_triggered();
    void on_modelsListWidget_itemDoubleClicked(QListWidgetItem* item);

    void updateVertexOnRAM(unsigned long long vertexOnRAM);
    void updateVertexOnVRAM(unsigned long long vertexOnVRAM);
    void updateModelLoading(Model3D* model, unsigned int progressValue);

    void modelWriterFinished();

private:
    Ui::MainWindow* ui;
    QSettings settings;

    Model3DWriter modelWriter;

    void keyPressEvent(QKeyEvent* e) override;
};

#endif // MAINWINDOW_H
