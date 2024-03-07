#pragma once

#include "drawable.h"
#include "meshdata/face.h"
#include "meshdata/halfedge.h"
#include "meshdata/vertex.h"
#include "smartpointerhelp.h"

#include <QFile>
#include <vector>

class Vertex;
class Face;
class HalfEdge;

struct PairHash {
  uint64_t operator()(const std::pair<Vertex *, Vertex *> p) const;
};

/**
 * Holds and manages vertex, face, and half-edge information for a custom mesh
 * object, generated from an OBJ file, and allows us to draw it in our scene.
 *
 * Exposes public methods to modify the mesh in useful ways.
 */
class Mesh : public Drawable {
public:
  // Constructs a Mesh instance from a given OBJ file
  Mesh(OpenGLContext *mp_context, QFile &file);
  virtual ~Mesh();

  void create() override;
  GLenum drawMode() override;

  /**
   * Split a given HalfEdge in two, adding and returning
   * a new vertex at the specified position.
   */
  Vertex *splitEdge(HalfEdge *edge, glm::vec3 pos);

  /**
   * Split a given HalfEdge in two, adding a new vertex
   *  at the exact center of the specified edge.
   */
  Vertex *splitEdge(HalfEdge *edge);

  void triangulateFace(Face *face); // Triangulate a given face.
  void catmullClarkSubdivide();     // Apply Catmull-Clark subdivision.

  void bindSkeleton(Joint *root);
  void unbindSkeleton();

private:
  std::vector<uPtr<Vertex>> verts;
  std::vector<uPtr<Face>> faces;
  std::vector<uPtr<HalfEdge>> edges;

  Joint *skeletonRoot;

  /**
   * Parses the provided .obj file, filling the provided vectors with mesh data.
   *
   * @param verts - vector of vertex positions to be filled
   * @param faces - face data, where each inner vector is 0-indexed vertices of
   * one face
   */
  static void parseOBJ(QFile &file, std::vector<glm::vec3> *verts,
                       std::vector<std::vector<int>> *faces);

  /**
   * Fills verts, faces, and edges using the given vertex/face information.
   *
   * @param verts - vector of vertex positions
   * @param faces - face data, where each inner vector is 0-indexed vertices of
   * one face
   */
  void buildMeshData(const std::vector<glm::vec3> &verts,
                     const std::vector<std::vector<int>> &faces);

  bool containsVertex(Vertex *vert)
      const; // Check to see if a Vertex pointer is part of this mesh.
  bool containsFace(
      Face *face) const; // Check to see if a Face pointer is part of this mesh.
  bool containsEdge(HalfEdge *edge)
      const; // Check to see if a HalfEdge pointer is part of this mesh.

  /**
   * Splits a given face into quadrangles. Primarily used for
   *  Catmull-Clark Subdivision. Takes the face to quadrangulate, the
   *  centroid vertex, and a vector of all edges pointing to new vertices.
   */
  void quadrangulateFace(Face *face, Vertex *centroid,
                         std::vector<HalfEdge *> &splitEdges);

  bool isBound() const;

  friend class MyGL;
};
