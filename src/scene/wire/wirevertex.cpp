#include "wirevertex.h"

WireVertex::WireVertex(OpenGLContext *context)
    : Drawable(context), vertex(nullptr) {}

void WireVertex::setVertex(Vertex *vert) { vertex = vert; }

void WireVertex::create() {
  if (!vertex) {
    return;
  }

  auto pos = glm::vec4(vertex->getPos(), 1);
  auto col = glm::vec4(1);

  count = 1;

  generatePos();
  mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
  mp_context->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4), &pos,
                           GL_STATIC_DRAW);

  generateCol();
  mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
  mp_context->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4), &col,
                           GL_STATIC_DRAW);
}

GLenum WireVertex::drawMode() { return GL_POINTS; }
