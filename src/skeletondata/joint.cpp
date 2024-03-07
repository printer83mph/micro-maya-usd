#include "joint.h"

#include "glm/ext/matrix_transform.hpp"
#include "utils.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>


int Joint::lastId = 0;

Joint::Joint(OpenGLContext* mp_context, QJsonObject json)
    : Drawable(mp_context), id(lastId++), parent(nullptr), children()
{
    name = json["name"].toString();
    setText(0, name);

    // position
    auto posArray = json["pos"].toArray();
    pos = glm::vec3(posArray[0].toDouble(),
                    posArray[1].toDouble(),
                    posArray[2].toDouble());

    // rotation
    auto rotArray = json["rot"].toArray();
    rot = glm::rotate<float>(glm::mat4(1.), rotArray[0].toDouble(),
        glm::vec3(rotArray[1].toDouble(), rotArray[2].toDouble(), rotArray[3].toDouble()));

    // add children
    auto childrenArray = json["children"].toArray();
    for (auto value : childrenArray)
    {
        auto obj = value.toObject();
        uPtr<Joint> newJoint = mkU<Joint>(mp_context, obj);

        newJoint->parent = this;
        addChild(newJoint.get());

        children.push_back(std::move(newJoint));
    }
}

Joint::~Joint()
{}

void Joint::create()
{
    createWithSelected(nullptr);
}

void Joint::createWithSelected(Joint* selected)
{
    // create new vectors
    std::vector<glm::vec4> pos, col;
    std::vector<GLuint> idx;

    // recursively build mesh
    recurseCreate(idx, pos, col, glm::mat4(1), selected);

    // VBO time!
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

GLenum Joint::drawMode()
{
    return GL_LINES;
}

glm::mat4 Joint::getLocalTransform() const
{
    return glm::translate(glm::mat4(1), pos) * rot;
}

glm::mat4 Joint::getOverallTransform() const
{
    // recursive case
    if (parent)
    {
        return parent->getOverallTransform() * getLocalTransform();
    }

    // base case (root node)
    return getLocalTransform();
}

void Joint::getAllJoints(std::vector<Joint*>& joints)
{
    joints.push_back(this);
    for (auto& j : children)
    {
        j->getAllJoints(joints);
    }
}

void Joint::getBindMatrices(std::array<glm::mat4, 100>& bindMats) const
{
    bindMats[id] = bind;

    for (auto& j : children)
    {
        j->getBindMatrices(bindMats);
    }
}

void Joint::getJointTransforms(std::array<glm::mat4, 100>& jointTransforms, glm::mat4 baseTransform) const
{
    glm::mat4 overall = baseTransform * getLocalTransform();

    jointTransforms[id] = overall;

    for (auto& j : children)
    {
        j->getJointTransforms(jointTransforms, overall);
    }
}

void Joint::generateBindMatrices(glm::mat4 baseTransform)
{
    glm::mat4 overall = baseTransform * getLocalTransform();

    bind = glm::inverse(overall);

    // calculate childrens' bind matrices
    for (auto& joint : children)
    {
        joint->generateBindMatrices(overall);
    }
}

void Joint::rotateLocal(float x, float y, float z)
{
    rot = rot * glm::eulerAngleXYZ(z, y, x);
}

int Joint::getId() const
{
    return id;
}

Joint* Joint::getParent()
{
    return parent;
}

void Joint::createEdgeCircle(std::vector<GLuint>& idx,
                                   std::vector<glm::vec4>& pos, std::vector<glm::vec4>& col,
                                   glm::vec3 axis, glm::vec3 color, glm::mat4& transform)
{
    glm::vec4 baseVector = glm::vec4(axis.y, axis.z, axis.x, 1);

    int startIdx = pos.size();
    for (int i = 0; i < 12; ++i)
    {
        auto rot = glm::rotate(glm::mat4(1.f), i * PI * 0.166667f, axis);
        pos.push_back(transform * rot * baseVector);
        col.push_back(glm::vec4(color, 0));
    }

    for (int i = 0; i < 12; ++i)
    {
        idx.push_back(startIdx + i);
        idx.push_back(startIdx + (i + 1) % 12);
    }
}

void Joint::recurseCreate(std::vector<GLuint>& idx,
                          std::vector<glm::vec4>& pos, std::vector<glm::vec4>& col,
                          glm::mat4 baseTransform, Joint* selected) const
{
    glm::mat4 overall = baseTransform * getLocalTransform();

    // make connections
    if (parent)
    {
        pos.push_back(baseTransform * glm::vec4(0, 0, 0, 1));
        col.push_back(glm::vec4(1, 0, 0, 0));
        idx.push_back(pos.size() - 1);

        pos.push_back(overall * glm::vec4(0, 0, 0, 1));
        col.push_back(glm::vec4(1, 1, 0, 0));
        idx.push_back(pos.size() - 1);
    }

    bool sel = this == selected;

    // make circles
    createEdgeCircle(idx, pos, col, glm::vec3(0.5f, 0, 0), sel ? glm::vec3(1.f) : glm::vec3(1.f, 0, 0), overall);
    createEdgeCircle(idx, pos, col, glm::vec3(0, 0.5f, 0), sel ? glm::vec3(1.f) : glm::vec3(0, 1.f, 0), overall);
    createEdgeCircle(idx, pos, col, glm::vec3(0, 0, 0.5f), sel ? glm::vec3(1.f) : glm::vec3(0, 0, 1.f), overall);

    for (auto& j : children)
    {
        j->recurseCreate(idx, pos, col, overall, selected);
    }
}

void Joint::createJointEdges(Joint& joint,
                             std::vector<GLuint>& idx,
                             std::vector<glm::vec4>& pos,
                             std::vector<glm::vec4>& col,
                             glm::mat4 baseTransform)
{
    glm::mat4 overall = baseTransform * joint.getLocalTransform();

    if (parent)
    {
        pos.push_back(baseTransform * glm::vec4(0, 0, 0, 1));
        col.push_back(glm::vec4(1, 0, 0, 0));
        idx.push_back(pos.size() - 1);

        pos.push_back(overall * glm::vec4(0, 0, 0, 1));
        col.push_back(glm::vec4(1, 1, 0, 0));
        idx.push_back(pos.size() - 1);
    }

    for (auto& j : joint.children)
    {
        createJointEdges(*j, idx, pos, col, overall);
    }
}
