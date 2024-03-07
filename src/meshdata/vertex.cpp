#include "vertex.h"

#include <limits>

int Vertex::nextId = 0;

Vertex::Vertex(glm::vec3 pos)
    : pos(pos), id(nextId++), joint1Idx(0), joint2Idx(0), joint1Weight(1.f),
      joint2Weight(0.f) {
  setText(QString::number(id));
}

Vertex::~Vertex() {}

glm::vec3 Vertex::getPos() const { return pos; }

HalfEdge *Vertex::getEdge() const { return edge; }

void Vertex::setPos(glm::vec3 p) { pos = p; }

void Vertex::assignWeights(std::vector<Joint *> &joints) {
  Joint *closest = joints[0];
  Joint *second = closest;
  float cl = std::numeric_limits<float>::max();
  float sn = cl;

  for (auto &joint : joints) {
    glm::vec3 jointPos =
        glm::vec3(joint->getOverallTransform() * glm::vec4(0, 0, 0, 1));
    auto diff = (pos - jointPos);
    float sqrDistance = glm::dot(diff, diff);

    if (sqrDistance < cl) {
      second = closest;
      sn = cl;
      closest = joint;
      cl = sqrDistance;
    } else if (sqrDistance < sn) {
      second = joint;
      sn = sqrDistance;
    }
  }

  float rootCl = glm::sqrt(cl);

  float w2 = rootCl / (rootCl + glm::sqrt(sn));

  setWeights(closest->getId(), second->getId(), 1.f - w2, w2);
}

void Vertex::clearWeights() { setWeights(0, 1, 1, 0); }

void Vertex::setWeights(int j1i, int j2i, float j1w, float j2w) {
  joint1Idx = j1i;
  joint2Idx = j2i;
  joint1Weight = j1w;
  joint2Weight = j2w;
}
