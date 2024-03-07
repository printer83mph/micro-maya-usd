#include "face.h"

#include <utils.h>

int Face::nextId = 0;

Face::Face() : color(utils::getRandomColor()), id(nextId++)
{
    setText(QString::number(id));
}

Face::Face(glm::vec3& color) : color(color), id(nextId++)
{
    setText(QString::number(id));
}

Face::~Face()
{}

glm::vec3 Face::getColor() const
{
    return color;
}

HalfEdge* Face::getEdge() const
{
    return edge;
}

int Face::getEdgeCount() const
{
    HalfEdge* e = edge;
    int count = 0;
    do
    {
        count++;
        e = e->getNextEdge();
    }
    while (e != edge);
    return count;
}

void Face::setColor(glm::vec3 col) {
    color = col;
}
