#include "wireface.h"

#include <vector>

#include <iostream>


WireFace::WireFace(OpenGLContext* context)
    : Drawable(context), face(nullptr)
{}

void WireFace::setFace(Face* f)
{
    face = f;
}

void WireFace::create()
{
    if (!face) {
        return;
    }

    std::vector<glm::vec4> pos, col;
    std::vector<GLuint> idx;
    auto color = glm::vec4(1) - glm::vec4(face->getColor(), 1);

    // first edge
    idx.push_back(0);

    pos.push_back(glm::vec4(face->getEdge()->getTailPos(), 1));
    col.push_back(color);

    HalfEdge* edge = face->getEdge()->getNextEdge();
    do
    {
        idx.push_back(pos.size());
        idx.push_back(pos.size());
        pos.push_back(glm::vec4(edge->getTailPos(), 1));
        col.push_back(color);

        edge = edge->getNextEdge();
    }
    while (edge != face->getEdge());

    // last point
    idx.push_back(0);

    count = idx.size();

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), pos.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_STATIC_DRAW);
}

GLenum WireFace::drawMode()
{
    return GL_LINE_STRIP;
}
