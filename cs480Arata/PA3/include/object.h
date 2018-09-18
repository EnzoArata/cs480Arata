#ifndef OBJECT_H
#define OBJECT_H

#include <vector>
#include "graphics_headers.h"
#define GLM_ENABLE_EXPERIMENTAL
class Engine;
class Object
{
  public:

    Object(Engine *engineReference);
    Object(Engine *engineReferenced, Object *parentReference);
    ~Object();
    void Update(unsigned int dt, glm::mat4 parentMatrix);
    void Render();
    void addChild(Object *childReference);
    void setScale(glm::vec3 newSize);
    Engine *engineRef;
    Object *parentRef;
    std::vector<Object*> childrenRef;
    glm::mat4 GetModel();
    glm::mat4 model;

    glm::mat4 parentModel;

  private:

    std::vector<Vertex> Vertices;
    std::vector<unsigned int> Indices;
    GLuint VB;
    GLuint IB;

    float angle;
    float angle2;
    float angleMultiplier;
    float angle2Multiplier;
    float rotationRate;
    float rotationRate2;

    glm::vec3 scaleValue;

};

#endif /* OBJECT_H */
