#ifndef OBJECT_H
#define OBJECT_H

#include <vector>
#include "graphics_headers.h"
#define GLM_ENABLE_EXPERIMENTAL
class Object
{
  public:
    Object();
    ~Object();
    void Update(unsigned int dt);
    void Render();

    glm::mat4 GetModel();

  private:
    glm::mat4 model;
    std::vector<Vertex> Vertices;
    std::vector<unsigned int> Indices;
    GLuint VB;
    GLuint IB;

    float angle;
};

#endif /* OBJECT_H */
