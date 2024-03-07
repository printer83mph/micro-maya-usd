#pragma once

#include "face.h"
#include "vertex.h"

#include <QListWidget>

class Vertex;
class Face;

class HalfEdge : public QListWidgetItem {
public:
  HalfEdge();
  virtual ~HalfEdge();

  glm::vec3 getTailPos() const;
  glm::vec3 getHeadPos() const;
  HalfEdge *getNextEdge() const;
  HalfEdge *getSymEdge() const;
  Face *getFace() const;
  Vertex *getNextVert() const;

private:
  HalfEdge *nextEdge; // The next half-edge in this loop
  HalfEdge *sym;      // Our symmetrical half-edge
  Face *face;         // The face this half-edge belongs to
  Vertex *nextVert;   // The vertex this points to
  const int id;       // Unique HalfEdge id

  static int nextId; // The next id to use

  friend class Mesh;
};
