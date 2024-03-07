#pragma once

#include "drawable.h"
#include "openglcontext.h"
#include "smartpointerhelp.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <QTreeWidget>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <array>
#include <vector>

class Joint : public Drawable, public QTreeWidgetItem {
public:
  // Constructs a structure of joints from a json object
  Joint(OpenGLContext *mp_context, QJsonObject json);
  ~Joint();

  void create() override;
  void createWithSelected(Joint *selected);

  GLenum drawMode() override; // Make sure we're in lines mode

  glm::mat4
  getLocalTransform() const; // Compound rotation -> translation in local space
  glm::mat4 getOverallTransform()
      const; // Total transformation matrix from world to local space

  void getAllJoints(std::vector<Joint *> &
                        joints); // fill a vector with this and all child joints

  void getBindMatrices(std::array<glm::mat4, 100> &bindMats)
      const; // fill an array with bind mats based on ids
  void getJointTransforms(std::array<glm::mat4, 100> &jointTransforms,
                          glm::mat4 baseTransform = glm::mat4(1))
      const; // fill an array with joint transforms based on ids

  // Recursively generates bind matrices downwards. Run when binding a mesh.
  void generateBindMatrices(glm::mat4 baseTransform);

  // rotate about our local axes
  void rotateLocal(float x, float y, float z);

  int getId() const;
  Joint *getParent();

private:
  QString name; // display name
  int id;       // id for use with skeleton shader

  static int lastId; // last used id for no collisions

  Joint *parent;                     // null if root node
  std::vector<uPtr<Joint>> children; // children vector (we own them HAHA)

  glm::vec3 pos;  // Position relative to parent joint
  glm::mat4 rot;  // Rotation relative to parent joint
  glm::mat4 bind; // Inverse of joint's compound transform matrix, generated
                  // once when binding mesh

  // Using an axis of rotation and a color, adds a loop of 12 edges to the given
  // idx, pos, and col vectors
  static void createEdgeCircle(std::vector<GLuint> &idx,
                               std::vector<glm::vec4> &pos,
                               std::vector<glm::vec4> &col, glm::vec3 axis,
                               glm::vec3 color, glm::mat4 &transform);

  // adds edge from joint to parent to idx, pos, col
  void recurseCreate(std::vector<GLuint> &idx, std::vector<glm::vec4> &pos,
                     std::vector<glm::vec4> &col, glm::mat4 baseTransform,
                     Joint *selected) const;

  // recursively draws edges to children
  void createJointEdges(Joint &joint, std::vector<GLuint> &idx,
                        std::vector<glm::vec4> &pos,
                        std::vector<glm::vec4> &col, glm::mat4 baseTransform);

  friend class Skeleton;
  friend class JointWidget;
};
