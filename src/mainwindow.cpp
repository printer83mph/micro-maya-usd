#include "mainwindow.h"
#include <ui_mainwindow.h>
#include "cameracontrolshelp.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->mygl->setFocus();

    // load OBJ button
    connect(ui->actionImportOBJ,
            &QAction::triggered,
            this,
            &MainWindow::slot_loadObj);
    // load skeleton button
    connect(ui->actionImportJSONSkeleton,
            &QAction::triggered,
            this,
            &MainWindow::slot_loadSkeleton);

    // ui initialization
    connect(ui->mygl,
            &MyGL::signal_clearUI,
            this,
            &MainWindow::slot_clearUI);

    connect(ui->mygl,
            &MyGL::signal_addVertex,
            this,
            &MainWindow::slot_addVertex);
    connect(ui->mygl,
            &MyGL::signal_addFace,
            this,
            &MainWindow::slot_addFace);
    connect(ui->mygl,
            &MyGL::signal_addEdge,
            this,
            &MainWindow::slot_addEdge);
    connect(ui->mygl,
            &MyGL::signal_setJoint,
            this,
            &MainWindow::slot_setJoint);

    // selecting vert, face, edge, or joint
    connect(ui->vertsListWidget,
            &QListWidget::itemClicked,
            this,
            &MainWindow::slot_setSelectedVertex);
    connect(ui->facesListWidget,
            &QListWidget::itemClicked,
            this,
            &MainWindow::slot_setSelectedFace);
    connect(ui->halfEdgesListWidget,
            &QListWidget::itemClicked,
            this,
            &MainWindow::slot_setSelectedEdge);
    connect(ui->jointsTreeWidget,
            &QTreeWidget::itemClicked,
            this,
            &MainWindow::slot_setSelectedJoint);
    // from mygl to here back to mygl
    connect(ui->mygl,
            &MyGL::signal_setSelectedVertex,
            this,
            &MainWindow::slot_setSelectedVertex);
    connect(ui->mygl,
            &MyGL::signal_setSelectedFace,
            this,
            &MainWindow::slot_setSelectedFace);
    connect(ui->mygl,
            &MyGL::signal_setSelectedEdge,
            this,
            &MainWindow::slot_setSelectedEdge);

    // set vertex position, face color
    connect(ui->vertPosXSpinBox,
            &QDoubleSpinBox::valueChanged,
            ui->mygl,
            &MyGL::slot_setVertPosX);
    connect(ui->vertPosYSpinBox,
            &QDoubleSpinBox::valueChanged,
            ui->mygl,
            &MyGL::slot_setVertPosY);
    connect(ui->vertPosZSpinBox,
            &QDoubleSpinBox::valueChanged,
            ui->mygl,
            &MyGL::slot_setVertPosZ);

    const float rotIncrement = 0.125f * PI;
    // update joint rotation
    connect(ui->jointRotXBackButton,
            &QPushButton::released,
            ui->mygl,
            [=]{ ui->mygl->rotateJoint(-rotIncrement, 0, 0); });
    connect(ui->jointRotXForButton,
            &QPushButton::released,
            ui->mygl,
            [=]{ ui->mygl->rotateJoint(rotIncrement, 0, 0); });
    connect(ui->jointRotYBackButton,
            &QPushButton::released,
            ui->mygl,
            [=]{ ui->mygl->rotateJoint(0, -rotIncrement, 0); });
    connect(ui->jointRotYForButton,
            &QPushButton::released,
            ui->mygl,
            [=]{ ui->mygl->rotateJoint(0, rotIncrement, 0); });
    connect(ui->jointRotZBackButton,
            &QPushButton::released,
            ui->mygl,
            [=]{ ui->mygl->rotateJoint(0, 0, -rotIncrement); });
    connect(ui->jointRotZForButton,
            &QPushButton::released,
            ui->mygl,
            [=]{ ui->mygl->rotateJoint(0, 0, rotIncrement); });

    connect(ui->faceRedSpinBox,
            &QDoubleSpinBox::valueChanged,
            ui->mygl,
            &MyGL::slot_setFaceRed);
    connect(ui->faceGreenSpinBox,
            &QDoubleSpinBox::valueChanged,
            ui->mygl,
            &MyGL::slot_setFaceGreen);
    connect(ui->faceBlueSpinBox,
            &QDoubleSpinBox::valueChanged,
            ui->mygl,
            &MyGL::slot_setFaceBlue);

    // mesh operations
    connect(ui->splitEdgeButton,
            &QPushButton::released,
            ui->mygl,
            &MyGL::slot_splitEdge);
    connect(ui->triangulateFaceButton,
            &QPushButton::released,
            ui->mygl,
            &MyGL::slot_triangulateFace);
    connect(ui->subdivideMeshButton,
            &QPushButton::released,
            ui->mygl,
            &MyGL::slot_subdivideMesh);

    // skeleton operations
    connect(ui->bindMeshButton,
            &QPushButton::released,
            ui->mygl,
            &MyGL::bindMesh);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::exit();
}

void MainWindow::on_actionCamera_Controls_triggered()
{
    CameraControlsHelp* c = new CameraControlsHelp();
    c->show();
}

void MainWindow::slot_loadObj()
{
    // TODO: this is inaccurate on Windows
    QString filePath = QFileDialog::getOpenFileName(this, "Select an OBJ file to load",
                                                    "./resources/obj_files",
                                                    "OBJ Files (*.obj)");

    QFile file = QFile(filePath);

    // make sure file is readable
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "File cannot be read", file.errorString());
        return;
    }

    // tell MyGL to load the file
    ui->mygl->loadObj(file);

    file.close();
}

void MainWindow::slot_loadSkeleton()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Select a JSON skeleton file to load",
                                                    "./resources/jsons",
                                                    "JSON Files (*.json)");

    QFile file = QFile(filePath);

    // make sure file is readable
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "File cannot be read", file.errorString());
        return;
    }

    auto jsonDoc = QJsonDocument().fromJson(file.readAll());
    ui->mygl->loadSkeleton(jsonDoc);
}

void MainWindow::slot_clearUI()
{
    ui->vertsListWidget->clear();
    ui->facesListWidget->clear();
    ui->halfEdgesListWidget->clear();

    updateVertPosSpinBoxes(glm::vec3(0));
    updateFaceColorSpinBoxes(glm::vec3(0));
}

void MainWindow::slot_addVertex(Vertex* vert)
{
    ui->vertsListWidget->addItem(vert);
}

void MainWindow::slot_addFace(Face* face)
{
    ui->facesListWidget->addItem(face);
}

void MainWindow::slot_addEdge(HalfEdge* edge)
{
    ui->halfEdgesListWidget->addItem(edge);
}

void MainWindow::slot_setJoint(Joint* joint)
{
    // TODO: clearing this might crash
    ui->jointsTreeWidget->clear();
    ui->jointsTreeWidget->addTopLevelItem(joint);
}

void MainWindow::slot_setSelectedVertex(QListWidgetItem* vert)
{
    if (!vert) {
        ui->mygl->clearSelectionMode();
        return;
    }

    // in case it's called from MyGL shortcuts
    updateSelectedVertex(vert);

    Vertex* vertex = static_cast<Vertex*>(vert);
    ui->mygl->setSelectedVertex(vertex);
    ui->mygl->update();

    auto pos = vertex->getPos();
    updateVertPosSpinBoxes(pos);
}

void MainWindow::slot_setSelectedFace(QListWidgetItem* fc)
{
    if (!fc) {
        ui->mygl->clearSelectionMode();
        return;
    }

    // in case it's called from MyGL shortcuts
    updateSelectedFace(fc);

    Face* face = static_cast<Face*>(fc);
    ui->mygl->setSelectedFace(face);
    ui->mygl->update();

    auto color = face->getColor();
    updateFaceColorSpinBoxes(color);
}

void MainWindow::slot_setSelectedEdge(QListWidgetItem* e)
{
    if (!e) {
        ui->mygl->clearSelectionMode();
        return;
    }

    // in case it's called from MyGL shortcuts
    updateSelectedEdge(e);

    HalfEdge* edge = static_cast<HalfEdge*>(e);
    ui->mygl->setSelectedEdge(edge);
    ui->mygl->update();
}

void MainWindow::slot_setSelectedJoint(QTreeWidgetItem* j)
{
    if (!j) {
        ui->mygl->clearSelectionMode();
        return;
    }

    Joint* joint = static_cast<Joint*>(j);
    ui->mygl->setSelectedJoint(joint);
    ui->mygl->update();
}

void MainWindow::updateVertPosSpinBoxes(glm::vec3 pos)
{
    ui->vertPosXSpinBox->blockSignals(true);
    ui->vertPosYSpinBox->blockSignals(true);
    ui->vertPosZSpinBox->blockSignals(true);
    ui->vertPosXSpinBox->setValue(pos.x);
    ui->vertPosYSpinBox->setValue(pos.y);
    ui->vertPosZSpinBox->setValue(pos.z);
    ui->vertPosXSpinBox->blockSignals(false);
    ui->vertPosYSpinBox->blockSignals(false);
    ui->vertPosZSpinBox->blockSignals(false);
}

void MainWindow::updateFaceColorSpinBoxes(glm::vec3 color)
{
    ui->faceRedSpinBox->blockSignals(true);
    ui->faceGreenSpinBox->blockSignals(true);
    ui->faceBlueSpinBox->blockSignals(true);
    ui->faceRedSpinBox->setValue(color.r);
    ui->faceGreenSpinBox->setValue(color.g);
    ui->faceBlueSpinBox->setValue(color.b);
    ui->faceRedSpinBox->blockSignals(false);
    ui->faceGreenSpinBox->blockSignals(false);
    ui->faceBlueSpinBox->blockSignals(false);
}

void MainWindow::updateSelectedVertex(QListWidgetItem* vert)
{
    ui->vertsListWidget->blockSignals(true);
    ui->vertsListWidget->setCurrentItem(vert);
    ui->vertsListWidget->blockSignals(false);
}

void MainWindow::updateSelectedFace(QListWidgetItem* face)
{
    ui->facesListWidget->blockSignals(true);
    ui->facesListWidget->setCurrentItem(face);
    ui->facesListWidget->blockSignals(false);
}

void MainWindow::updateSelectedEdge(QListWidgetItem* edge)
{
    ui->halfEdgesListWidget->blockSignals(true);
    ui->halfEdgesListWidget->setCurrentItem(edge);
    ui->halfEdgesListWidget->blockSignals(false);
}
