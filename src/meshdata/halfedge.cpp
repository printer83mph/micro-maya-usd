#include "halfedge.h"

int HalfEdge::nextId = 0;

HalfEdge::HalfEdge() : id(nextId++)
{
    setText(QString::number(id));
}

HalfEdge::~HalfEdge()
{}

glm::vec3 HalfEdge::getTailPos() const
{
    return nextVert->getPos();
}

glm::vec3 HalfEdge::getHeadPos() const
{
    return sym->nextVert->getPos();
}

HalfEdge* HalfEdge::getNextEdge() const
{
    return nextEdge;
}

HalfEdge* HalfEdge::getSymEdge() const
{
    return sym;
}

Face* HalfEdge::getFace() const
{
    return face;
}

Vertex* HalfEdge::getNextVert() const
{
    return nextVert;
}
