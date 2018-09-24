#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <iostream>
using namespace std;


#include "graphics_headers.h"
#include "camera.h"
#include "shader.h"
#include "object.h"
#include "engine.h"
#define GLM_ENABLE_EXPERIMENTAL
class Engine;
class Graphics
{
  public:
    Graphics();
    Graphics(Engine *engineRef);
    ~Graphics();
    bool Initialize(int width, int height);
    void Update(unsigned int dt);
    void Render();
    Engine *m_engine;

  private:
    std::string ErrorString(GLenum error);

    Camera *m_camera;
    Shader *m_shader;


    GLint m_projectionMatrix;
    GLint m_viewMatrix;
    GLint m_modelMatrix;

    Object *m_cube;
    Object *m_cubeMoon;
};

#endif /* GRAPHICS_H */
