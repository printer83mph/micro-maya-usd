#pragma once

#include "drawable.h"
#include "meshdata/vertex.h"
#include "openglcontext.h"

class WireVertex : public Drawable {
public:
  WireVertex(OpenGLContext *context);

  void setVertex(Vertex *vert);

  void create() override;
  GLenum drawMode() override;

private:
  Vertex *vertex;
};
