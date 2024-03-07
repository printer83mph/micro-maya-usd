#pragma once

#include "halfedge.h"

#include <glm/glm.hpp>
#include <QListWidget>

class HalfEdge;

class Face : public QListWidgetItem
{
public:
    Face();                  // Constructs a face with a random color.
    Face(glm::vec3& color);  // Constructs a face with the specified color.
    virtual ~Face();

    glm::vec3 getColor() const;
    HalfEdge* getEdge() const;
    int getEdgeCount() const;

    void setColor(glm::vec3 col);

private:
    HalfEdge* edge;     // One half-edge associated with this face
    glm::vec3 color;    // This face's RGB color
    const int id;       // Unique face id

    static int nextId;  // The next id to use

    friend class Mesh;
};
