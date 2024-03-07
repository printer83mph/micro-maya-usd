#include "mesh.h"

#include "utils.h"

#include <pxr/usd/usdGeom/mesh.h>

#include <unordered_map>
#include <unordered_set>
#include <vector>

uint64_t PairHash::operator()(const std::pair<Vertex *, Vertex *> p) const {
  uint64_t addr1 = reinterpret_cast<uint64_t>(p.first);
  uint64_t addr2 = reinterpret_cast<uint64_t>(p.second);
  return addr1 ^ addr2;
}

Mesh::Mesh(OpenGLContext *mp_context, QFile &file) : Drawable(mp_context) {

  // parse the file, fill these vectors
  std::vector<glm::vec3> fileVerts;
  std::vector<std::vector<int>> fileFaces;
  parseOBJ(file, &fileVerts, &fileFaces);

  // now we build the data structure.
  buildMeshData(fileVerts, fileFaces);
}

Mesh::~Mesh() {}

void Mesh::create() {
  // create new vectors
  std::vector<glm::vec4> pos, nor, col;
  std::vector<GLuint> idx;

  // skeleton stuff
  std::vector<glm::ivec2> ids;
  std::vector<glm::vec2> weights;

  int totalVerts = 0;
  for (auto &face : faces) {
    // grab our face's associated half-edge
    HalfEdge *prevEdge = face->edge;
    HalfEdge *currEdge = prevEdge->nextEdge;
    int faceVerts = 0;

    // loop through half-edges
    do {
      // add position
      pos.push_back(glm::vec4(currEdge->nextVert->pos, 1));

      // add normal
      glm::vec3 &prevVert = prevEdge->nextVert->pos;
      glm::vec3 &currVert = currEdge->nextVert->pos;
      glm::vec3 &nextVert = currEdge->nextEdge->nextVert->pos;

      // TODO: this may be the wrong calculation
      nor.push_back(glm::vec4(
          glm::normalize(glm::cross(currVert - prevVert, nextVert - currVert)),
          0));

      // add color
      col.push_back(glm::vec4(face->color, 0));

      // joint stuff
      if (isBound()) {
        ids.push_back(glm::ivec2(currEdge->nextVert->joint1Idx,
                                 currEdge->nextVert->joint2Idx));
        weights.push_back(glm::vec2(currEdge->nextVert->joint1Weight,
                                    currEdge->nextVert->joint2Weight));
      }

      // increment edge and num of face verts
      prevEdge = currEdge;
      currEdge = currEdge->nextEdge;
      ++faceVerts;

    } while (prevEdge != face->edge);

    // add indices
    for (int i = 0; i < faceVerts - 2; ++i) {
      idx.push_back(totalVerts);
      idx.push_back(totalVerts + i + 1);
      idx.push_back(totalVerts + i + 2);
    }

    totalVerts += faceVerts;
  }

  // VBO time!
  count = idx.size();

  generateIdx();
  mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
  mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint),
                           idx.data(), GL_STATIC_DRAW);

  generatePos();
  mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
  mp_context->glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4),
                           pos.data(), GL_STATIC_DRAW);

  generateNor();
  mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufNor);
  mp_context->glBufferData(GL_ARRAY_BUFFER, nor.size() * sizeof(glm::vec4),
                           nor.data(), GL_STATIC_DRAW);

  generateCol();
  mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
  mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4),
                           col.data(), GL_STATIC_DRAW);

  if (isBound()) {
    generateJointIdx();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufJointIdx);
    mp_context->glBufferData(GL_ARRAY_BUFFER, ids.size() * sizeof(glm::ivec2),
                             &ids[0], GL_STATIC_DRAW);

    generateJointWgt();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufJointWgt);
    mp_context->glBufferData(GL_ARRAY_BUFFER,
                             weights.size() * sizeof(glm::vec2), &weights[0],
                             GL_STATIC_DRAW);
  }
}

GLenum Mesh::drawMode() { return GL_TRIANGLES; }

Vertex *Mesh::splitEdge(HalfEdge *edge, glm::vec3 pos) {
  // make sure edge is in this mesh
  if (!containsEdge(edge)) {
    // TODO: this will crash the program LOL
    return nullptr;
  }

  HalfEdge *sym = edge->sym;

  // get average point
  uPtr<Vertex> newVert = mkU<Vertex>(pos);

  uPtr<HalfEdge> newEdge = mkU<HalfEdge>();
  newEdge->face = edge->face;
  newEdge->nextEdge = edge->nextEdge;
  newEdge->nextVert = edge->nextVert;
  newEdge->sym = sym;

  uPtr<HalfEdge> newSymEdge = mkU<HalfEdge>();
  newSymEdge->face = sym->face;
  newSymEdge->nextEdge = sym->nextEdge;
  newSymEdge->nextVert = sym->nextVert;
  newSymEdge->sym = edge;

  newVert->edge = edge;
  edge->nextVert->edge = newEdge.get();
  sym->nextVert->edge = newSymEdge.get();

  edge->nextVert = newVert.get();
  edge->nextEdge = newEdge.get();
  edge->sym = newSymEdge.get();

  sym->nextVert = newVert.get();
  sym->nextEdge = newSymEdge.get();
  sym->sym = newEdge.get();

  Vertex *newVertPtr = newVert.get();
  verts.push_back(std::move(newVert));
  edges.push_back(std::move(newEdge));
  edges.push_back(std::move(newSymEdge));

  return newVertPtr;
}

Vertex *Mesh::splitEdge(HalfEdge *edge) {
  return splitEdge(edge,
                   (edge->nextVert->pos + edge->sym->nextVert->pos) * 0.5f);
}

void Mesh::triangulateFace(Face *face) {
  // make sure edge is in this mesh
  if (!containsFace(face)) {
    return;
  }

  // don't do anything for triangle
  int edgeCount = face->getEdgeCount();
  if (edgeCount == 3) {
    return;
  }

  HalfEdge *rootPrevEdge = face->edge; // edge connecting to rootVert
  while (rootPrevEdge->nextEdge != face->edge) {
    rootPrevEdge = rootPrevEdge->nextEdge;
  }

  Vertex *rootVert = face->edge->sym->nextVert; // vert all new edges attach to

  HalfEdge *prevEdge = face->edge;
  HalfEdge *edge = face->edge->nextEdge;
  for (int i = 0; i < edgeCount - 3; ++i) {
    HalfEdge *originalNext = edge->nextEdge;

    Vertex *connectVert =
        edge->nextVert; // vert these specific new edges attach to

    uPtr<Face> newFace = mkU<Face>();
    uPtr<HalfEdge> newEdge = mkU<HalfEdge>();
    uPtr<HalfEdge> newSymEdge = mkU<HalfEdge>();

    newEdge->nextEdge = prevEdge;
    newEdge->nextVert = rootVert;

    newSymEdge->nextEdge = edge->nextEdge;
    newSymEdge->nextVert = connectVert;
    newSymEdge->face = face;

    newEdge->sym = newSymEdge.get();
    newSymEdge->sym = newEdge.get();

    edge->nextEdge = newEdge.get();
    rootPrevEdge->nextEdge = newSymEdge.get();

    newFace->edge = newEdge.get();
    face->edge = newSymEdge.get();

    // assign new face to all right side edges
    HalfEdge *iterEdge = edge;
    do {
      iterEdge->face = newFace.get();
      iterEdge = iterEdge->nextEdge;
    } while (iterEdge != edge);

    faces.push_back(std::move(newFace));
    edges.push_back(std::move(newEdge));
    edges.push_back(std::move(newSymEdge));

    // iterate
    prevEdge = face->edge;
    edge = originalNext;
  }
}

void Mesh::catmullClarkSubdivide() {
  int initialVertCount = verts.size();
  int initialHalfEdgeCount = edges.size();
  int initialFaceCount = faces.size();

  // map of centroid vertices
  std::unordered_map<Face *, Vertex *> centroids;

  // make centroids
  for (auto &face : faces) {
    // find average position
    glm::vec3 avg;
    int count = 0;
    HalfEdge *edge = face->edge;
    do {
      avg += edge->nextVert->pos;
      count++;
      edge = edge->nextEdge;
    } while (edge != face->edge);
    avg /= (float)count;

    verts.push_back(mkU<Vertex>(avg));
    centroids[face.get()] = verts.back().get();
  }

  // split edges, store pointers to edges
  std::vector<HalfEdge *> midpointEdges;

  // (sym) edges we've already split
  std::unordered_set<HalfEdge *> finishedEdges;

  for (int ei = 0; ei < initialHalfEdgeCount; ++ei) {
    auto &edge = edges[ei];
    // if we haven't yet done this edge
    if (finishedEdges.find(edge.get()) == finishedEdges.end()) {
      finishedEdges.insert(edge->sym);
    } else { // otherwise we skip this bc weve already done stuff w this
      continue;
    }

    bool hasTwoFaces = edge->sym->face;

    glm::vec3 faceSum =
        centroids[edge->face]->pos +
        (hasTwoFaces ? centroids[edge->sym->face]->pos : glm::vec3(0));

    glm::vec3 midpointPos =
        (edge->getHeadPos() + edge->getTailPos() + faceSum) /
        (hasTwoFaces ? 4.f : 3.f);

    // store pointers to edges pointed at new vert
    midpointEdges.push_back(edge.get());
    midpointEdges.push_back(edge->sym);

    splitEdge(edge.get(), midpointPos);
  }

  // smooth original verts
  for (int vi = 0; vi < initialVertCount; ++vi) {
    auto &vert = verts[vi];

    // collect adjacent things
    glm::vec3 midpointAndFaceSum;
    int adjMidpointCount = 0;
    HalfEdge *edge = vert->edge;
    do {
      // TODO: fix for non-manifold bugs
      edge = edge->nextEdge;
      midpointAndFaceSum += (edge->nextVert->pos + centroids[edge->face]->pos);
      adjMidpointCount++;
      edge = edge->sym;
    } while (edge != vert->edge);

    vert->pos =
        ((adjMidpointCount - 2.f) / (float)adjMidpointCount) * vert->pos +
        midpointAndFaceSum / (float)(adjMidpointCount * adjMidpointCount);
  }

  // quadrangulate faces
  for (int fi = 0; fi < initialFaceCount; fi++) {
    auto &face = faces[fi];

    // grab all split edges
    std::vector<HalfEdge *> splitEdges;

    HalfEdge *startEdge = face->edge;
    do {
      splitEdges.push_back(startEdge);
      startEdge = startEdge->nextEdge->nextEdge;
    } while (startEdge != face->edge);

    // quadrangulate!
    quadrangulateFace(face.get(), centroids[face.get()], splitEdges);
  }
}

void Mesh::bindSkeleton(Joint *root) {
  if (skeletonRoot) {
    unbindSkeleton();
  }
  skeletonRoot = root;

  // assign weights to joints
  std::vector<Joint *> joints;
  skeletonRoot->getAllJoints(joints);
  for (auto &vert : verts) {
    vert->assignWeights(joints);
  }

  // VBO time!
  create();
}

void Mesh::unbindSkeleton() {
  skeletonRoot = nullptr;

  for (auto &vert : verts) {
    vert->clearWeights();
  }
}

pxr::UsdGeomMesh Mesh::createUsdMesh(pxr::UsdStagePtr stage,
                                     const char *path) const {
  std::vector<glm::vec4> points;
  std::vector<int> indices;

  for (auto &face : faces) {
    int offset = points.size();
    int faceEdgeCount = 0;
    auto iterEdge = face->edge;
    do {
      points.push_back(glm::vec4(iterEdge->nextVert->pos, 1));
      iterEdge = iterEdge->nextEdge;
      ++faceEdgeCount;
    } while (iterEdge != face->edge);
    for (int i = 1; i + 1 < faceEdgeCount; ++i) {
      indices.push_back(offset);
      indices.push_back(offset + i);
      indices.push_back(offset + i + 1);
    }
  }

  pxr::UsdGeomMesh usdMesh =
      pxr::UsdGeomMesh::Define(stage, pxr::SdfPath(path));

  auto pointsAttr = usdMesh.GetPointsAttr();
  pointsAttr.Set(points);
  auto idxAttr = usdMesh.GetFaceVertexIndicesAttr();
  idxAttr.Set(indices);

  return usdMesh;
}

void Mesh::parseOBJ(QFile &file, std::vector<glm::vec3> *verts,
                    std::vector<std::vector<int>> *faces) {
  QTextStream in(&file);

  while (!in.atEnd()) {
    QString line = in.readLine();
    QStringList unfilteredWords = line.split(" ");

    // only look at words with content, filter out extra whitespace
    QStringList words;
    for (auto &word : unfilteredWords) {
      if (word.size() > 0) {
        words.push_back(word);
      }
    }

    if (words.size() == 0) {
      continue;
    }

    // we have a vertex
    if (words[0] == QString("v")) {
      words.removeFirst();
      glm::vec3 pos;
      for (unsigned int i = 0; i < 3; ++i) {
        pos[i] = words[i].toFloat();
      }
      verts->push_back(pos);
      continue;
    }

    // we have a face
    if (words[0] == QString("f")) {
      words.removeFirst();
      std::vector<int> vertIndices;
      for (auto &word : words) {
        QStringList parts = word.split("/");
        vertIndices.push_back(parts[0].toInt() - 1);
      }
      faces->push_back(vertIndices);
      continue;
    }
  }
}

void Mesh::buildMeshData(const std::vector<glm::vec3> &fileVerts,
                         const std::vector<std::vector<int>> &fileFaces) {
  // fill verts
  for (auto &vertPos : fileVerts) {
    verts.push_back(mkU<Vertex>(vertPos));
  }

  // fill faces and half-edges
  for (auto &vertIdxs : fileFaces) {
    auto face = mkU<Face>();

    // setup first edge, pointing to first vertex
    auto firstEdge = mkU<HalfEdge>();
    firstEdge->face = face.get();
    face->edge = firstEdge.get();
    firstEdge->nextVert = verts[vertIdxs[0]].get();
    verts[vertIdxs[0]]->edge = firstEdge.get();

    HalfEdge *lastEdge = firstEdge.get();
    edges.push_back(std::move(firstEdge));

    for (unsigned int i = 1; i < vertIdxs.size(); ++i) {
      auto edge = mkU<HalfEdge>();
      // point current edge to current vertex, face to face
      edge->nextVert = verts[vertIdxs[i]].get();
      verts[vertIdxs[i]]->edge = edge.get();
      // set face
      edge->face = face.get();
      // set prev edge next to this guy
      lastEdge->nextEdge = edge.get();
      lastEdge = edge.get();
      edges.push_back(std::move(edge));
    }

    // connect last edge to first
    lastEdge->nextEdge = face->edge;

    faces.push_back(std::move(face));
  }

  // hash map bullshit
  std::unordered_map<std::pair<Vertex *, Vertex *>, HalfEdge *, PairHash>
      edgeIndices;

  // assign symmetrical edges
  for (auto &face : faces) {
    HalfEdge *prevEdge = face->edge;
    HalfEdge *edge = prevEdge->nextEdge;

    do {
      // pair for hashing
      auto pair = std::make_pair(prevEdge->nextVert, edge->nextVert);

      // if the pair is not already in:
      if (edgeIndices.find(pair) == edgeIndices.end()) {
        edgeIndices[std::make_pair(edge->nextVert, prevEdge->nextVert)] = edge;
      } else // the pair is there!
      {
        auto symEdge = edgeIndices[pair];
        symEdge->sym = edge;
        edge->sym = symEdge;

        // remove from map
        edgeIndices.erase(pair);
      }

      // traverse
      prevEdge = edge;
      edge = edge->nextEdge;
    } while (prevEdge != face->edge);
  }

  // make loose symmetrical edges
  for (auto &face : faces) {
    HalfEdge *prevEdge = face->edge;
    HalfEdge *edge = prevEdge->nextEdge;

    // assign opposite direction half-edge for loose edges
    do {
      // check if pair still in hashmap
      const auto pair = std::make_pair(edge->nextVert, prevEdge->nextVert);

      if (edgeIndices.find(pair) != edgeIndices.end()) {
        uPtr<HalfEdge> looseSymEdge = mkU<HalfEdge>();
        looseSymEdge->nextVert = prevEdge->nextVert;

        // set sym pointers
        looseSymEdge->sym = edge;
        edge->sym = looseSymEdge.get();

        // add to edges vector
        edges.push_back(std::move(looseSymEdge));
      }

      prevEdge = edge;
      edge = edge->nextEdge;
    } while (prevEdge != face->edge);
  }
}

bool Mesh::containsVertex(Vertex *vert) const {
  for (auto &v : verts) {
    if (vert == v.get()) {
      return true;
    }
  }
  return false;
}
bool Mesh::containsFace(Face *face) const {
  for (auto &f : faces) {
    if (face == f.get()) {
      return true;
    }
  }
  return false;
}
bool Mesh::containsEdge(HalfEdge *edge) const {
  for (auto &e : edges) {
    if (edge == e.get()) {
      return true;
    }
  }
  return false;
}

void Mesh::quadrangulateFace(Face *face, Vertex *centroid,
                             std::vector<HalfEdge *> &splitEdges) {
  // store non-split edges
  auto lastEdges = std::vector<HalfEdge *>();
  for (auto &edge : splitEdges) {
    lastEdges.push_back(edge->nextEdge);
  }

  // make new edges
  for (unsigned int ei = 0; ei < splitEdges.size(); ++ei) {
    HalfEdge *edge = splitEdges[ei];

    uPtr<HalfEdge> newInEdge = mkU<HalfEdge>();
    uPtr<HalfEdge> newOutEdge = mkU<HalfEdge>();

    newInEdge->face = face;
    newOutEdge->face = face;

    edge->nextEdge = newInEdge.get();
    newInEdge->nextEdge = newOutEdge.get();
    newOutEdge->nextEdge =
        lastEdges[((int)ei - 1 + splitEdges.size()) % splitEdges.size()];

    newInEdge->nextVert = centroid;
    newOutEdge->nextVert =
        splitEdges[((int)ei - 1 + splitEdges.size()) % splitEdges.size()]
            ->nextVert;

    centroid->edge = newInEdge.get();

    edges.push_back(std::move(newInEdge));
    edges.push_back(std::move(newOutEdge));
  }

  // assign sym pointers
  for (unsigned int ei = 0; ei < splitEdges.size(); ++ei) {
    HalfEdge *inEdge = splitEdges[ei]->nextEdge;
    HalfEdge *outEdge =
        splitEdges[((int)ei + 1) % splitEdges.size()]->nextEdge->nextEdge;

    inEdge->sym = outEdge;
    outEdge->sym = inEdge;
  }

  face->edge = splitEdges[0];

  for (unsigned int i = 1; i < splitEdges.size(); ++i) {
    glm::vec3 newCol =
        glm::clamp(face->color + (utils::getRandomColor() * 0.3f - 0.15f),
                   glm::vec3(0), glm::vec3(1));
    uPtr<Face> newFace = mkU<Face>(newCol);

    newFace->edge = splitEdges[i];
    HalfEdge *currEdge = splitEdges[i];
    do {
      currEdge->face = newFace.get();
      currEdge = currEdge->nextEdge;
    } while (currEdge != splitEdges[i]);

    faces.push_back(std::move(newFace));
  }
}

bool Mesh::isBound() const { return !!skeletonRoot; }
