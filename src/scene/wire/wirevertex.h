#pragma once

#include <drawable.h>
#include <openglcontext.h>
#include <meshdata/vertex.h>


class WireVertex : public Drawable
{
public:
    WireVertex(OpenGLContext* context);

    void setVertex(Vertex* vert);

    void create() override;
    GLenum drawMode() override;

private:
    Vertex* vertex;
};
