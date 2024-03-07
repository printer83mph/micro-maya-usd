#pragma once

#include "halfedge.h"
#include "skeletondata/joint.h"

#include <QListWidget>
#include <glm/glm.hpp>

class HalfEdge;

class Vertex : public QListWidgetItem {
public:
  Vertex(glm::vec3 pos);
  virtual ~Vertex();

  glm::vec3 getPos() const;
  HalfEdge *getEdge() const;

  void setPos(glm::vec3 p);

  // automatically assign weights by distance
  void assignWeights(std::vector<Joint *> &joints);
  void clearWeights();

private:
  // half-edge mesh data
  glm::vec3 pos;  // This vertex's position
  HalfEdge *edge; // Pointer to a half-edge which points to this vertex
  const int id;   // Unique vertex id

  // joint weight
  int joint1Idx, joint2Idx;
  float joint1Weight, joint2Weight;

  static int nextId; // The next id to use

  void setWeights(int j1i, int j2i, float j1w, float j2w);

  friend class Mesh;
};
