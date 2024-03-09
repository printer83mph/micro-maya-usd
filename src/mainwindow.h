#pragma once

#include "meshdata/face.h"
#include "meshdata/halfedge.h"
#include "meshdata/vertex.h"
#include "skeletondata/joint.h"

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private slots:
  void on_actionQuit_triggered();

  void on_actionCamera_Controls_triggered();

  void slot_loadObj();
  void slot_loadSkeleton();
  void slot_exportUSD();

  // UI management called from MyGL
  void slot_clearUI();
  void slot_addVertex(Vertex *vert);
  void slot_addFace(Face *face);
  void slot_addEdge(HalfEdge *edge);
  void slot_setJoint(Joint *joint);

  void slot_setSelectedVertex(QListWidgetItem *vert);
  void slot_setSelectedFace(QListWidgetItem *face);
  void slot_setSelectedEdge(QListWidgetItem *edge);
  void slot_setSelectedJoint(QTreeWidgetItem *joint);

private:
  Ui::MainWindow *ui;

  void updateVertPosSpinBoxes(glm::vec3 pos);
  void updateFaceColorSpinBoxes(glm::vec3 color);

  void updateSelectedVertex(QListWidgetItem *vert);
  void updateSelectedFace(QListWidgetItem *face);
  void updateSelectedEdge(QListWidgetItem *edge);
};
