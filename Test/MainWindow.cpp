#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , settings("settings.ini", QSettings::IniFormat, this)
    , modelWriter(this)
{
    ui->setupUi(this);
    connect(ui->openGLWidget->getEngine().getMainCamera(), SIGNAL(objectChanged()), ui->openGLWidget, SLOT(update()));
    connect(ui->viewDistanceDoubleSpinBox, SIGNAL(valueChanged(double)), &ui->openGLWidget->getEngine(), SLOT(setViewDistance(double)));
    connect(ui->viewDistanceCheckBox, SIGNAL(toggled(bool)), &ui->openGLWidget->getEngine(), SLOT(setViewDistanceEnabled(bool)));
    connect(ui->waitLoadingCheckBox, SIGNAL(toggled(bool)), &ui->openGLWidget->getEngine(), SLOT(setWaitLoading(bool)));
    connect(ui->limitMaxVertexCheckBox, SIGNAL(toggled(bool)), &ui->openGLWidget->getEngine(), SLOT(setLimitMaxVertexEnabled(bool)));
    connect(ui->limitMaxVertexDoubleSpinBox, SIGNAL(valueChanged(double)), &ui->openGLWidget->getEngine(), SLOT(setLimitMaxVertex(double)));
    connect(&modelWriter, SIGNAL(finished()), this, SLOT(modelWriterFinished()));

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

void MainWindow::on_actionSaveFile_triggered()
{
    if (ui->modelsListWidget->currentItem())
    {
        QString fileName = settings.value("ModelFileSave", QString()).toString();
        QStringList filterList = { "Modèle PTS (*.pts)", "Modèle BIN (*.bin)", "Modèle BINI (*.bini)", "Modèle OCT (*.oct)", "Modèle OCTI (*.octi)" };
        QString selectedFilter;
        for (QString filter : filterList)
            if (QFileInfo(fileName).suffix() == filter.split("*.")[1].remove(')')) selectedFilter = filter;
        
        QString filters = filterList[0];
        for (unsigned int i = 1; i < filterList.count(); i++) filters += " ;; " + filterList[i];

        fileName = QFileDialog::getSaveFileName(this, "Ouvrir modèle 3D", fileName, filters, &selectedFilter);
        if (!fileName.isEmpty())
        {
            bool yes = true;
            if (QFileInfo(fileName).suffix().contains("oct") && QFile::exists(QFileInfo(fileName).path() + "/listOctree.txt"))
            {
                if (QMessageBox::question(this, "Fichier listOctree.txt déjà existant", "Voulez-vous écraser listOctree.txt ?") != QMessageBox::Yes)
                    yes = false;
            }
            if(yes)
            {
                settings.setValue("ModelFileSave", fileName);
                settings.sync();

                ui->openGLWidget->getEngine().lockModelsUpdater();
                QEventLoop loop(this);

                QThread* loader = QThread::create(&Model3DWriter::write, &modelWriter, ui->openGLWidget->getEngine().getModel(ui->modelsListWidget->currentRow()), fileName);
                connect(loader, SIGNAL(finished()), &loop, SLOT(quit()));
                loader->start();
                loop.exec();

                delete loader;

                ui->openGLWidget->getEngine().unlockModelsUpdater();
                //Model3DWriter::write(ui->openGLWidget->getEngine().getModel(ui->modelsListWidget->currentRow()), fileName);
            }
        }
    }
}

void MainWindow::on_modelsListWidget_itemDoubleClicked(QListWidgetItem* item)
{
    ui->openGLWidget->getEngine().getMainCamera()->lookAt(ui->openGLWidget->getEngine().getModel(ui->modelsListWidget->row(item)));
}

void MainWindow::updateVertexOnRAM(unsigned long long vertexOnRAM)
{
    QString value = QString::number(vertexOnRAM);
    for (int i = value.count() - 3; i >= 1; i -= 3) value.insert(i, ' ');
    ui->vertexOnRAMLabel->setText(value);
}

void MainWindow::updateVertexOnVRAM(unsigned long long vertexOnVRAM)
{
    QString value = QString::number(vertexOnVRAM);
    for (int i = value.count() - 3; i >= 1; i -= 3) value.insert(i, ' ');
    ui->vertexOnVRAMLabel->setText(value);
}

void MainWindow::updateModelLoading(Model3D* model, unsigned int progressValue)
{
    int index;
    for (index = 0; index < ui->modelsListWidget->count() && ui->openGLWidget->getEngine().getModel(index) != model; index++);

    ui->modelsListWidget->item(index)->setText(model->getName() + (progressValue < 100 ? " (" + QString::number(progressValue) + "%)" : ""));
}

void MainWindow::modelWriterFinished()
{
    QMessageBox::information(this, "Ecriture du modèle", "L'écriture du modèle est terminée !");
}

void MainWindow::keyPressEvent(QKeyEvent* e)
{
    if (e->modifiers().testFlag(Qt::ControlModifier) && e->key() == Qt::Key_W)
    {
        if (ui->modelsListWidget->currentItem() != nullptr)
        {
            ui->openGLWidget->getEngine().closeModel(ui->modelsListWidget->currentRow());
            ui->modelsListWidget->takeItem(ui->modelsListWidget->currentRow());
        }
        e->accept();
    }
    else if (e->modifiers().testFlag(Qt::ControlModifier) && e->key() == Qt::Key_U)
    {
        ui->openGLWidget->update();
        e->accept();
    }
    else
        e->ignore();
}