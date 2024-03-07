#pragma once

#include "drawable.h"
#include "la.h"

#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLShaderProgram>

class SquarePlane : public Drawable {
public:
  SquarePlane(OpenGLContext *mp_context);
  virtual void create();
};
