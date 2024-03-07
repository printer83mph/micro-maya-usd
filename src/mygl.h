#pragma once

#include "camera.h"
#include "openglcontext.h"
#include "scene/mesh.h"
#include "scene/wire/wireedge.h"
#include "scene/wire/wireface.h"
#include "scene/wire/wirevertex.h"
#include "shaderprogram.h"
#include "smartpointerhelp.h"

#include <QFile>
#include <QJsonDocument>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

enum SelectionMode { NONE, VERTEX, FACE, EDGE, JOINT };

class MyGL : public OpenGLContext {
  Q_OBJECT
public:
  explicit MyGL(QWidget *parent = nullptr);
  ~MyGL();

  void initializeGL();
  void resizeGL(int w, int h);
  void paintGL();

  void loadObj(QFile &file);
  void loadSkeleton(QJsonDocument &doc);
  void bindMesh();

  void clearSelectionMode();
  void setSelectedVertex(Vertex *vert);
  void setSelectedFace(Face *face);
  void setSelectedEdge(HalfEdge *edge);
  void setSelectedJoint(Joint *joint);

  void rotateJoint(float x, float y, float z);

protected:
  void keyPressEvent(QKeyEvent *e);

signals:
  void signal_clearUI();
  void signal_addVertex(Vertex *vert);
  void signal_addFace(Face *face);
  void signal_addEdge(HalfEdge *edge);
  void signal_setJoint(Joint *joint);

  void signal_setSelectedVertex(QListWidgetItem *vert);
  void signal_setSelectedFace(QListWidgetItem *face);
  void signal_setSelectedEdge(QListWidgetItem *edge);

public slots:
  void slot_setVertPosX(double x);
  void slot_setVertPosY(double y);
  void slot_setVertPosZ(double z);
  void slot_setFaceRed(double r);
  void slot_setFaceGreen(double g);
  void slot_setFaceBlue(double b);

  void slot_splitEdge();
  void slot_triangulateFace();
  void slot_subdivideMesh();

private:
  uPtr<Mesh> m_mesh;       // Our custom mesh instance
  uPtr<Joint> m_rootJoint; // Our JSON-loaded skeleton
  WireVertex m_wireVert;   // Wire vert display instance
  WireFace m_wireFace;     // Wire face display instance
  WireEdge m_wireEdge;     // Wire edge display instance
  ShaderProgram
      m_progLambert;        // A shader program that uses lambertian reflection
  ShaderProgram m_progFlat; // A shader program that uses "flat" reflection (no
                            // shadowing at all)
  ShaderProgram m_progSkeleton; // Skeleton shader program

  GLuint vao; // A handle for our vertex array object. This will store the VBOs
              // created in our geometry classes. Don't worry too much about
              // this. Just know it is necessary in order to render geometry.

  Camera m_glCamera;

  Vertex *selectedVert;
  Face *selectedFace;
  HalfEdge *selectedEdge;
  Joint *selectedJoint;

  SelectionMode selectMode;

  void populateUI(); // Emits signals to populate MainWindow QListWidgets with
                     // mesh items.
  void createMeshVBOs(); // Runs create methods of mesh object and mesh display
                         // objects.
};
