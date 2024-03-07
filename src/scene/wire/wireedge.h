#pragma once

#include "drawable.h"
#include "meshdata/halfedge.h"
#include "openglcontext.h"

class WireEdge : public Drawable {
public:
  WireEdge(OpenGLContext *context);

  void setEdge(HalfEdge *edge);

  void create() override;
  GLenum drawMode() override;

private:
  HalfEdge *edge;
};
