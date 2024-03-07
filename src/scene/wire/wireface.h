#pragma once

#include <drawable.h>
#include <openglcontext.h>
#include <meshdata/face.h>


class WireFace : public Drawable
{
public:
    WireFace(OpenGLContext* context);

    void setFace(Face* vert);

    void create() override;
    GLenum drawMode() override;

private:
    Face* face;
};
