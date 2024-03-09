#include "mygl.h"
#include "glm/fwd.hpp"

#include <la.h>

#include <QApplication>
#include <QKeyEvent>
#include <pxr/usd/usd/stage.h>

#include <filesystem>
#include <string>

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent), m_mesh(nullptr), m_rootJoint(nullptr),
      m_wireVert(this), m_wireFace(this), m_wireEdge(this), m_progLambert(this),
      m_progFlat(this), m_progSkeleton(this), m_glCamera(),
      m_lastMousePos(0, 0), selectMode(SelectionMode::NONE) {
  setFocusPolicy(Qt::StrongFocus);
  setMouseTracking(true);
}

MyGL::~MyGL() {
  makeCurrent();
  glDeleteVertexArrays(1, &vao);
  if (m_mesh) {
    m_mesh->destroy();
  }
  m_wireVert.destroy();
  m_wireFace.destroy();
  m_wireEdge.destroy();
}

void MyGL::initializeGL() {
  // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
  // If you were programming in a non-Qt context you might use GLEW (GL
  // Extension Wrangler)instead
  initializeOpenGLFunctions();
  // Print out some information about the current OpenGL context
  debugContextVersion();

  // Set a few settings/modes in OpenGL rendering
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POLYGON_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
  // Set the size with which points should be rendered
  glPointSize(5);
  // Set the color with which the screen is filled at the start of each render
  // call.
  glClearColor(0.5, 0.5, 0.5, 1);

  printGLErrorLog();

  // Create a Vertex Attribute Object
  glGenVertexArrays(1, &vao);

  // Create and set up the diffuse shader
  m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
  // Create and set up the flat lighting shader
  m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
  // Create and set up the skeleton shader
  m_progSkeleton.create(":/glsl/skeleton.vert.glsl",
                        ":/glsl/skeleton.frag.glsl");

  // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
  // using multiple VAOs, we can just bind one once.
  glBindVertexArray(vao);
}

void MyGL::resizeGL(int w, int h) {
  // This code sets the concatenated view and perspective projection matrices
  // used for our scene's camera view.
  m_glCamera = Camera(w, h);
  glm::mat4 viewproj = m_glCamera.getViewProj();

  // Upload the view-projection matrix to our shaders (i.e. onto the graphics
  // card)

  m_progLambert.setViewProjMatrix(viewproj);
  m_progFlat.setViewProjMatrix(viewproj);
  m_progSkeleton.setViewProjMatrix(viewproj);

  printGLErrorLog();
}

// This function is called by Qt any time your GL window is supposed to update
// For example, when the function update() is called, paintGL is called
// implicitly.
void MyGL::paintGL() {
  // Clear the screen so that we only see newly drawn images
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  m_progFlat.setViewProjMatrix(m_glCamera.getViewProj());
  m_progLambert.setViewProjMatrix(m_glCamera.getViewProj());
  m_progLambert.setCamPos(m_glCamera.eye);
  m_progSkeleton.setViewProjMatrix(m_glCamera.getViewProj());
  m_progSkeleton.setCamPos(m_glCamera.eye);
  m_progFlat.setModelMatrix(glm::mat4(1.f));
  m_progLambert.setModelMatrix(glm::mat4(1.f));
  m_progSkeleton.setModelMatrix(glm::mat4(1.f));

  if (m_mesh) {
    if (m_mesh->isBound()) {
      m_progSkeleton.draw(*m_mesh);
    } else {
      m_progLambert.draw(*m_mesh);
    }
  }

  // selection visualization
  glDisable(GL_DEPTH_TEST);

  if (m_rootJoint) {
    m_progFlat.draw(*m_rootJoint);
  }

  m_progFlat.setModelMatrix(glm::mat4(1));
  switch (selectMode) {
  case SelectionMode::VERTEX:
    m_progFlat.draw(m_wireVert);
    break;
  case SelectionMode::FACE:
    m_progFlat.draw(m_wireFace);
    break;
  case SelectionMode::EDGE:
    m_progFlat.draw(m_wireEdge);
    break;
  default:
    // selection mode NONE or JOINT
    break;
  }

  glEnable(GL_DEPTH_TEST);
}

void MyGL::loadObj(QFile &file) {
  clearSelectionMode();

  if (m_mesh) {
    m_mesh->destroy();
  }

  m_mesh = mkU<Mesh>(this, file);
  m_mesh->create();

  // clear and initialize ui
  emit signal_clearUI();
  populateUI();

  update();
}

void MyGL::loadSkeleton(const QJsonDocument &doc) {
  if (m_mesh) {
    m_mesh->unbindSkeleton();
  }

  m_rootJoint = mkU<Joint>(this, doc.object()["root"].toObject());
  m_rootJoint->create();

  emit signal_setJoint(m_rootJoint.get());
}

void MyGL::exportUSD(const QString &filePath) const {
  auto path = std::filesystem::path(filePath.toStdString());
  auto stage = pxr::UsdStage::CreateNew(path.c_str());

  auto meshPath = "/" + path.replace_extension("").filename().string();
  auto mesh = m_mesh->createUsdMesh(stage, meshPath.c_str());

  stage->SetDefaultPrim(mesh.GetPrim());
  stage->Save();
}

void MyGL::bindMesh() {
  if (!m_mesh || !m_rootJoint) {
    return;
  }

  // generate bind matrices
  m_rootJoint->generateBindMatrices(glm::mat4(1));

  // get get bind matrices and give to shaders
  std::array<glm::mat4, 100> bindMats, jointTransforms;
  m_rootJoint->getBindMatrices(bindMats);
  m_rootJoint->getJointTransforms(jointTransforms);

  m_progSkeleton.setBindMats(bindMats);
  m_progSkeleton.setJointTfms(jointTransforms);

  // assign vertex weights
  m_mesh->bindSkeleton(m_rootJoint.get());
}

void MyGL::clearSelectionMode() {
  selectMode = SelectionMode::NONE;
  selectedVert = nullptr;
  selectedEdge = nullptr;
  selectedFace = nullptr;
  m_wireVert.setVertex(nullptr);
  m_wireFace.setFace(nullptr);
  m_wireEdge.setEdge(nullptr);
}

void MyGL::setSelectedVertex(Vertex *vert) {
  clearSelectionMode();
  selectMode = SelectionMode::VERTEX;

  selectedVert = vert;
  m_wireVert.setVertex(vert);
  m_wireVert.create();
}

void MyGL::setSelectedFace(Face *face) {
  clearSelectionMode();
  selectMode = SelectionMode::FACE;

  selectedFace = face;
  m_wireFace.setFace(face);
  m_wireFace.create();
}

void MyGL::setSelectedEdge(HalfEdge *edge) {
  clearSelectionMode();
  selectMode = SelectionMode::EDGE;

  selectedEdge = edge;
  m_wireEdge.setEdge(edge);
  m_wireEdge.create();
}

void MyGL::setSelectedJoint(Joint *joint) {
  clearSelectionMode();
  selectMode = SelectionMode::JOINT;

  selectedJoint = joint;
  m_rootJoint->createWithSelected(selectedJoint);
}

void MyGL::rotateJoint(float x, float y, float z) {
  if (!selectedJoint) {
    return;
  }

  // rotate joint and generate skeleton VBOs
  selectedJoint->rotateLocal(x, y, z);
  m_rootJoint->createWithSelected(selectedJoint);

  // update skeleton shader's joint transforms array
  std::array<glm::mat4, 100> jointTransforms;
  m_rootJoint->getJointTransforms(jointTransforms, glm::mat4(1));
  m_progSkeleton.setJointTfms(jointTransforms);

  update();
}

bool MyGL::isMeshLoaded() const { return m_mesh != nullptr; }

void MyGL::keyPressEvent(QKeyEvent *e) {
  float amount = 2.0f;
  if (e->modifiers() & Qt::ShiftModifier) {
    amount = 10.0f;
  }
  // http://doc.qt.io/qt-5/qt.html#Key-enum
  if (e->key() == Qt::Key_Escape) {
    QApplication::quit();
  } else if (e->key() == Qt::Key_Right) {
    m_glCamera.RotateAboutUp(-amount);
  } else if (e->key() == Qt::Key_Left) {
    m_glCamera.RotateAboutUp(amount);
  } else if (e->key() == Qt::Key_Up) {
    m_glCamera.RotateAboutRight(-amount);
  } else if (e->key() == Qt::Key_Down) {
    m_glCamera.RotateAboutRight(amount);
  } else if (e->key() == Qt::Key_1) {
    m_glCamera.fovy += amount;
  } else if (e->key() == Qt::Key_2) {
    m_glCamera.fovy -= amount;
  } else if (e->key() == Qt::Key_W) {
    m_glCamera.TranslateAlongLook(amount, false);
  } else if (e->key() == Qt::Key_S) {
    m_glCamera.TranslateAlongLook(-amount, false);
  } else if (e->key() == Qt::Key_D) {
    m_glCamera.TranslateAlongRight(amount);
  } else if (e->key() == Qt::Key_A) {
    m_glCamera.TranslateAlongRight(-amount);
  } else if (e->key() == Qt::Key_Q) {
    m_glCamera.TranslateAlongUp(-amount);
  } else if (e->key() == Qt::Key_E) {
    m_glCamera.TranslateAlongUp(amount);
  } else if (e->key() == Qt::Key_R) {
    m_glCamera = Camera(this->width(), this->height());
  } else if (e->key() == Qt::Key_N && selectedEdge) {
    // next edge
    emit signal_setSelectedEdge(selectedEdge->getNextEdge());
  } else if (e->key() == Qt::Key_M && selectedEdge) {
    // sym edge
    emit signal_setSelectedEdge(selectedEdge->getSymEdge());
  } else if (e->key() == Qt::Key_F && selectedEdge) {
    // face of edge
    emit signal_setSelectedFace(selectedEdge->getFace());
  } else if (e->key() == Qt::Key_V && selectedEdge) {
    // vert of edge
    emit signal_setSelectedVertex(selectedEdge->getNextVert());
  } else if (e->key() == Qt::Key_H && selectedVert) {
    // edge of vert
    emit signal_setSelectedEdge(selectedVert->getEdge());
  } else if (e->keyCombination().keyboardModifiers().testFlag(
                 Qt::ShiftModifier) &&
             e->key() == Qt::Key_H && selectedFace) {
    // edge of face
    setSelectedEdge(selectedFace->getEdge());
  }
  m_glCamera.RecomputeAttributes();
  update(); // Calls paintGL, among other things
}

void MyGL::mouseMoveEvent(QMouseEvent *e) {
  auto newPos = glm::ivec2(e->pos().x(), e->pos().y());
  glm::vec2 delta = newPos - m_lastMousePos;
  delta /= devicePixelRatio();

  if (e->buttons().testFlag(Qt::LeftButton)) {
    m_glCamera.RotateAboutUp(-delta.x * 0.5);
    m_glCamera.RotateAboutRight(-delta.y * 0.5);
    m_glCamera.RecomputeAttributes();
    update();
  } else if (e->buttons().testFlag(Qt::RightButton)) {
    m_glCamera.ZoomByRatio(1 - delta.y * 0.005);
    m_glCamera.RecomputeAttributes();
    update();
  } else if (e->buttons().testFlag(Qt::MiddleButton)) {
    // TODO: test with mouse
    m_glCamera.TranslateAlongRight(delta.x * 0.01);
    m_glCamera.TranslateAlongUp(delta.y * 0.01);
  }

  m_lastMousePos = newPos;
}

void MyGL::slot_setVertPosX(double x) {
  if (!selectedVert) {
    return;
  }
  glm::vec3 newPos = selectedVert->getPos();
  newPos.x = x;
  selectedVert->setPos(newPos);

  createMeshVBOs();
  update();
}

void MyGL::slot_setVertPosY(double y) {
  if (!selectedVert) {
    return;
  }
  glm::vec3 newPos = selectedVert->getPos();
  newPos.y = y;
  selectedVert->setPos(newPos);

  createMeshVBOs();
  update();
}

void MyGL::slot_setVertPosZ(double z) {
  if (!selectedVert) {
    return;
  }
  glm::vec3 newPos = selectedVert->getPos();
  newPos.z = z;
  selectedVert->setPos(newPos);

  createMeshVBOs();
  update();
}

void MyGL::slot_setFaceRed(double r) {
  if (!selectedFace) {
    return;
  }
  glm::vec3 newCol = selectedFace->getColor();
  newCol.r = r;
  selectedFace->setColor(newCol);

  createMeshVBOs();
  update();
}

void MyGL::slot_setFaceGreen(double g) {
  if (!selectedFace) {
    return;
  }
  glm::vec3 newCol = selectedFace->getColor();
  newCol.g = g;
  selectedFace->setColor(newCol);

  createMeshVBOs();
  update();
}

void MyGL::slot_setFaceBlue(double b) {
  if (!selectedFace) {
    return;
  }
  glm::vec3 newCol = selectedFace->getColor();
  newCol.b = b;
  selectedFace->setColor(newCol);

  createMeshVBOs();
  update();
}

void MyGL::slot_splitEdge() {
  if (!selectedEdge) {
    return;
  }

  Vertex *newVert = m_mesh->splitEdge(selectedEdge);

  populateUI();
  emit signal_setSelectedVertex(newVert);
}

void MyGL::slot_triangulateFace() {
  if (!selectedFace) {
    return;
  }

  m_mesh->triangulateFace(selectedFace);

  populateUI();
  clearSelectionMode();

  createMeshVBOs();
  update();
}

void MyGL::slot_subdivideMesh() {
  if (!m_mesh) {
    return;
  }
  m_mesh->catmullClarkSubdivide();

  populateUI();
  clearSelectionMode();

  createMeshVBOs();
  update();
}

void MyGL::createMeshVBOs() {
  m_mesh->create();
  m_wireVert.create();
  m_wireFace.create();
  m_wireEdge.create();
}

void MyGL::populateUI() {
  for (auto &vert : m_mesh->verts) {
    emit signal_addVertex(vert.get());
  }
  for (auto &face : m_mesh->faces) {
    emit signal_addFace(face.get());
  }
  for (auto &edge : m_mesh->edges) {
    // don't show loose half-edges
    // TODO: maybe dont worry about this kind of thing
    if (!edge->getNextEdge()) {
      continue;
    }
    emit signal_addEdge(edge.get());
  }
}
