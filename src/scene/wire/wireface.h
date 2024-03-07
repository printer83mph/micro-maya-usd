#pragma once

#include "drawable.h"
#include "meshdata/face.h"
#include "openglcontext.h"

class WireFace : public Drawable {
public:
  WireFace(OpenGLContext *context);

  void setFace(Face *vert);

  void create() override;
  GLenum drawMode() override;

private:
  Face *face;
};
