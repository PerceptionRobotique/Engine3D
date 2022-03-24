#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , settings("settings.ini", QSettings::IniFormat, this)
{
    ui->setupUi(this);
    connect(ui->openGLWidget->getEngine().getMainCamera(), SIGNAL(objectChanged()), ui->openGLWidget, SLOT(update()));
    connect(ui->viewDistanceDoubleSpinBox, SIGNAL(valueChanged(double)), &ui->openGLWidget->getEngine(), SLOT(setViewDistance(double)));
    connect(ui->viewDistanceCheckBox, SIGNAL(toggled(bool)), &ui->openGLWidget->getEngine(), SLOT(setViewDistanceEnabled(bool)));
    connect(ui->waitLoadingCheckBox, SIGNAL(toggled(bool)), &ui->openGLWidget->getEngine(), SLOT(setWaitLoading(bool)));

    restoreGeometry(settings.value("WindowGeometry").toByteArray());
    restoreState(settings.value("WindowState").toByteArray());
}

MainWindow::~MainWindow()
{
    settings.setValue("WindowGeometry", saveGeometry());
    settings.setValue("WindowState", saveState());
    delete ui;
}

void MainWindow::on_actionOpenFile_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Ouvrir modèle 3D", settings.value("ModelFileOpen", QString()).toString(), "Modèle 3D (*.pts *.bin *.bini *.oct *.octi)");
    if (!fileName.isEmpty())
    {
        settings.setValue("ModelFileOpen", fileName);
        ui->openGLWidget->getEngine().openModel(fileName);
        ui->openGLWidget->getEngine().getModels().last()->setRotationX(-90.0f);
        connect(ui->openGLWidget->getEngine().getModels().last(), SIGNAL(modelLoadingUpdate(Model3D*, unsigned int)), this, SLOT(updateModelLoading(Model3D*, unsigned int)));
        connect(ui->openGLWidget->getEngine().getModels().last(), SIGNAL(vertexOnRAMChanged(unsigned long long)), this, SLOT(updateVertexOnRAM(unsigned long long)));
        connect(ui->openGLWidget->getEngine().getModels().last(), SIGNAL(vertexOnVRAMChanged(unsigned long long)), this, SLOT(updateVertexOnVRAM(unsigned long long)));

        ui->modelsListWidget->addItem(ui->openGLWidget->getEngine().getModels().last()->getName());
    }
}

void MainWindow::on_modelsListWidget_itemDoubleClicked(QListWidgetItem* item)
{
    ui->openGLWidget->getEngine().getMainCamera()->lookAt(ui->openGLWidget->getEngine().getModel(ui->modelsListWidget->row(item)));
}

void MainWindow::updateVertexOnRAM(unsigned long long vertexOnRAM)
{
    ui->vertexOnRAMLabel->setText(QString::number(vertexOnRAM));
}

void MainWindow::updateVertexOnVRAM(unsigned long long vertexOnVRAM)
{
    ui->vertexOnVRAMLabel->setText(QString::number(vertexOnVRAM));
}

void MainWindow::updateModelLoading(Model3D* model, unsigned int progressValue)
{
    int index;
    for (index = 0; index < ui->modelsListWidget->count() && ui->openGLWidget->getEngine().getModel(index) != model; index++);

    ui->modelsListWidget->item(index)->setText(model->getName() + (progressValue < 100 ? " (" + QString::number(progressValue) + "%)" : ""));
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->modifiers().testFlag(Qt::ControlModifier) && event->key() == Qt::Key_W)
    {
        if (ui->modelsListWidget->currentItem() != nullptr)
        {
            ui->openGLWidget->getEngine().closeModel(ui->modelsListWidget->currentRow());
            ui->modelsListWidget->takeItem(ui->modelsListWidget->currentRow());
        }
        event->accept();
    }
    else if (event->modifiers().testFlag(Qt::ControlModifier) && event->key() == Qt::Key_U)
    {
        ui->openGLWidget->update();
        event->accept();
    }
    else
        event->ignore();
}