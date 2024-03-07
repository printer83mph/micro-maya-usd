#pragma once

#include <drawable.h>
#include <openglcontext.h>
#include <meshdata/halfedge.h>


class WireEdge : public Drawable
{
public:
    WireEdge(OpenGLContext* context);

    void setEdge(HalfEdge* edge);

    void create() override;
    GLenum drawMode() override;

private:
    HalfEdge* edge;
};
