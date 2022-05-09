#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , settings("settings.ini", QSettings::IniFormat, this)
    
{
    ui->setupUi(this);

#ifdef GRAPHICSVIEW_H
    connect(ui->viewDistanceDoubleSpinBox, SIGNAL(valueChanged(double)), ui->graphicsView->getEngine(), SLOT(setViewDistance(double)));
    connect(ui->viewDistanceCheckBox, SIGNAL(toggled(bool)), ui->graphicsView->getEngine(), SLOT(setViewDistanceEnabled(bool)));
    connect(ui->waitLoadingCheckBox, SIGNAL(toggled(bool)), ui->graphicsView->getEngine(), SLOT(setWaitLoading(bool)));
    connect(ui->limitMaxVertexCheckBox, SIGNAL(toggled(bool)), ui->graphicsView->getEngine(), SLOT(setMaxVertexLimitEnabled(bool)));
    connect(ui->limitMaxVertexSpinBox, SIGNAL(valueChanged(int)), ui->graphicsView->getEngine(), SLOT(setMaxVertexLimit(int)));
#else
    connect(ui->viewDistanceDoubleSpinBox, SIGNAL(valueChanged(double)), ui->openGLWidget->getEngine(), SLOT(setViewDistance(double)));
    connect(ui->viewDistanceCheckBox, SIGNAL(toggled(bool)), ui->openGLWidget->getEngine(), SLOT(setViewDistanceEnabled(bool)));
    connect(ui->waitLoadingCheckBox, SIGNAL(toggled(bool)), ui->openGLWidget->getEngine(), SLOT(setWaitLoading(bool)));
    connect(ui->limitMaxVertexCheckBox, SIGNAL(toggled(bool)), ui->openGLWidget->getEngine(), SLOT(setMaxVertexLimitEnabled(bool)));
    connect(ui->limitMaxVertexSpinBox, SIGNAL(valueChanged(int)), ui->openGLWidget->getEngine(), SLOT(setMaxVertexLimit(int)));
#endif

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

#ifdef GRAPHICSVIEW_H
        ui->graphicsView->getEngine()->openModel(fileName);
        ui->graphicsView->getEngine()->getModels().last()->setRotationX(-90.0f);
        connect(ui->graphicsView->getEngine()->getModels().last(), SIGNAL(modelLoadingUpdate(Model3D*, unsigned int)), this, SLOT(updateModelLoading(Model3D*, unsigned int)));
        connect(ui->graphicsView->getEngine()->getModels().last(), SIGNAL(vertexOnRAMChanged()), this, SLOT(updateVertexOnRAM()));
        connect(ui->graphicsView->getEngine()->getModels().last(), SIGNAL(vertexOnVRAMChanged()), this, SLOT(updateVertexOnVRAM()));
        ui->modelsListWidget->addItem(ui->graphicsView->getEngine()->getModels().last()->getName());
#else
        ui->openGLWidget->getEngine()->openModel(fileName);
        ui->openGLWidget->getEngine()->getModels().last()->setRotationX(-90.0f);
        connect(ui->openGLWidget->getEngine()->getModels().last(), SIGNAL(modelLoadingUpdate(Model3D*, unsigned int)), this, SLOT(updateModelLoading(Model3D*, unsigned int)));
        connect(ui->openGLWidget->getEngine()->getModels().last(), SIGNAL(vertexOnRAMChanged()), this, SLOT(updateVertexOnRAM()));
        connect(ui->openGLWidget->getEngine()->getModels().last(), SIGNAL(vertexOnVRAMChanged()), this, SLOT(updateVertexOnVRAM()));
        ui->modelsListWidget->addItem(ui->openGLWidget->getEngine()->getModels().last()->getName());
#endif
    }
}

void MainWindow::on_modelsListWidget_itemDoubleClicked(QListWidgetItem* item)
{
#ifdef GRAPHICSVIEW_H
    ui->graphicsView->getEngine()->getMainCamera()->lookAt(ui->graphicsView->getEngine()->getModel(ui->modelsListWidget->row(item)));
#else
    ui->openGLWidget->getEngine()->getMainCamera()->lookAt(ui->openGLWidget->getEngine()->getModel(ui->modelsListWidget->row(item)));
#endif
}

void MainWindow::on_captureFramePushButton_clicked()
{
#ifdef GRAPHICSVIEW_H
    ui->graphicsView->grabImage().save("test.png");
#else
    ui->openGLWidget->grabImage().save("test.png");
#endif
    QMessageBox::information(this, "Capturer une image", "Image capturée.");
}

void MainWindow::updateModelLoading(Model3D* model, unsigned int progressValue)
{
    int index;
#ifdef GRAPHICSVIEW_H
    for (index = 0; index < ui->modelsListWidget->count() && ui->graphicsView->getEngine()->getModel(index) != model; index++);
#else
    for (index = 0; index < ui->modelsListWidget->count() && ui->openGLWidget->getEngine()->getModel(index) != model; index++);
#endif

    ui->modelsListWidget->item(index)->setText(model->getName() + (progressValue < 100 ? " (" + QString::number(progressValue) + "%)" : ""));
}

void MainWindow::updateVertexOnRAM()
{
    QString value = QString::number(Model3D::getVertexOnRAM());
    for (int i = value.count() - 3; i >= 1; i -= 3) value.insert(i, ' ');
    ui->vertexOnRAMLabel->setText(value);
}

void MainWindow::updateVertexOnVRAM()
{
    QString value = QString::number(Model3D::getVertexOnVRAM());
    for (int i = value.count() - 3; i >= 1; i -= 3) value.insert(i, ' ');
    ui->vertexOnVRAMLabel->setText(value);
}

void MainWindow::keyPressEvent(QKeyEvent* e)
{
    if (e->modifiers().testFlag(Qt::ControlModifier) && e->key() == Qt::Key_W)
    {
        if (ui->modelsListWidget->currentItem() != nullptr)
        {
#ifdef GRAPHICSVIEW_H
            ui->graphicsView->getEngine()->closeModel(ui->modelsListWidget->currentRow());
#else
            ui->openGLWidget->getEngine()->closeModel(ui->modelsListWidget->currentRow());
#endif
            ui->modelsListWidget->takeItem(ui->modelsListWidget->currentRow());
        }
        e->accept();
    }
    else if (e->modifiers().testFlag(Qt::ControlModifier) && e->key() == Qt::Key_U)
    {

        e->accept();
    }
    else
        e->ignore();
}