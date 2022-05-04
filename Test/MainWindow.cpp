#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , settings("settings.ini", QSettings::IniFormat, this)
    
{
    ui->setupUi(this);
    
    connect(ui->viewDistanceDoubleSpinBox, SIGNAL(valueChanged(double)), ui->graphicsView->getEngine(), SLOT(setViewDistance(double)));
    connect(ui->viewDistanceCheckBox, SIGNAL(toggled(bool)), ui->graphicsView->getEngine(), SLOT(setViewDistanceEnabled(bool)));
    connect(ui->waitLoadingCheckBox, SIGNAL(toggled(bool)), ui->graphicsView->getEngine(), SLOT(setWaitLoading(bool)));
    connect(ui->limitMaxVertexCheckBox, SIGNAL(toggled(bool)), ui->graphicsView->getEngine(), SLOT(setMaxVertexLimitEnabled(bool)));
    connect(ui->limitMaxVertexSpinBox, SIGNAL(valueChanged(int)), ui->graphicsView->getEngine(), SLOT(setMaxVertexLimit(int)));

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
        ui->graphicsView->getEngine()->openModel(fileName);
        ui->graphicsView->getEngine()->getModels().last()->setRotationX(-90.0f);
        connect(ui->graphicsView->getEngine()->getModels().last(), SIGNAL(modelLoadingUpdate(Model3D*, unsigned int)), this, SLOT(updateModelLoading(Model3D*, unsigned int)));
        connect(ui->graphicsView->getEngine()->getModels().last(), SIGNAL(vertexOnRAMChanged()), this, SLOT(updateVertexOnRAM()));
        connect(ui->graphicsView->getEngine()->getModels().last(), SIGNAL(vertexOnVRAMChanged()), this, SLOT(updateVertexOnVRAM()));

        ui->modelsListWidget->addItem(ui->graphicsView->getEngine()->getModels().last()->getName());
    }
}

void MainWindow::on_modelsListWidget_itemDoubleClicked(QListWidgetItem* item)
{
    ui->graphicsView->getEngine()->getMainCamera()->lookAt(ui->graphicsView->getEngine()->getModel(ui->modelsListWidget->row(item)));
}

void MainWindow::updateModelLoading(Model3D* model, unsigned int progressValue)
{
    int index;
    for (index = 0; index < ui->modelsListWidget->count() && ui->graphicsView->getEngine()->getModel(index) != model; index++);

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
            ui->graphicsView->getEngine()->closeModel(ui->modelsListWidget->currentRow());
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