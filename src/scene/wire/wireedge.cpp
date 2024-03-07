#include "wireedge.h"

WireEdge::WireEdge(OpenGLContext *context) : Drawable(context), edge(nullptr) {}

void WireEdge::setEdge(HalfEdge *e) { edge = e; }

void WireEdge::create() {
  if (!edge) {
    return;
  }

  std::array<glm::vec4, 2> pos, col;

  pos[0] = glm::vec4(edge->getHeadPos(), 1);
  pos[1] = glm::vec4(edge->getTailPos(), 1);

  col[0] = glm::vec4(1, 0, 0, 0);
  col[1] = glm::vec4(1, 1, 0, 0);

  count = pos.size();

  generatePos();
  mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
  mp_context->glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4),
                           pos.data(), GL_STATIC_DRAW);

  generateCol();
  mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
  mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4),
                           col.data(), GL_STATIC_DRAW);
}

GLenum WireEdge::drawMode() { return GL_LINES; }
