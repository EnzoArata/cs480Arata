#include "graphics.h"

Graphics::Graphics()
{
  timeScale = 1;
  orbitScale = 2.5;
}

Graphics::~Graphics()
{

}

bool Graphics::Initialize(int width, int height)
{
  // Used for the linux OS
  #if !defined(__APPLE__) && !defined(MACOSX)
    // cout << glewGetString(GLEW_VERSION) << endl;
    glewExperimental = GL_TRUE;

    auto status = glewInit();

    // This is here to grab the error that comes from glew init.
    // This error is an GL_INVALID_ENUM that has no effects on the performance
    glGetError();

    //Check for error
    if (status != GLEW_OK)
    {
      std::cerr << "GLEW Error: " << glewGetErrorString(status) << "\n";
      return false;
    }
  #endif

  // For OpenGL 3
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Init Camera
  m_camera = new Camera();
  if(!m_camera->Initialize(width, height))
  {
    printf("Camera Failed to Initialize\n");
    return false;
  }

  // Create the object
  m_cube = new Object(MODEL_DIR + "sphere.obj", 0, 1, 1, 10);

  m_cube->AddChild(MODEL_DIR + "2kMercury.jpg", 70, 1.6, .017, .4);
  m_cube->AddChild(MODEL_DIR + "2kVenus.jpg", 110, 1.17, .004, .9);

  m_cube->AddChild(MODEL_DIR + "2kEarthDay.jpg", 150, 1, 1, 1)->AddChild(MODEL_DIR + "2kMoon.jpg", 7, 1, 1, 0.3);

  Object *temp = m_cube->AddChild(MODEL_DIR + "2kMars.jpg", 200, .80, .96, .5);
  for(int i = 0; i < 2; i++){
    temp->AddChild(MODEL_DIR + "2kMoon.jpg", 7+i*3, 1, 1, 0.3);
  }

  //m_cube->AddChild(MODEL_DIR + "2kMars.jpg", 200, 1, .5);
  temp = m_cube->AddChild(MODEL_DIR + "2kJupiter.jpg", 800, .43, 2.4, 11.2);
  /*for(int i = 0; i<62; i++){
    temp->AddChild(MODEL_DIR + "2kMoon.jpg", 15+i*4, 1, 1, 0.3);
  }*/
  m_cube->AddChild(MODEL_DIR + "2kSaturn.jpg", 1500, .32, 2.18,  9.4)->AddRing(1, 9.4);
  m_cube->AddChild(MODEL_DIR + "2kUranus.jpg", 2900, .22, 1.41, 4);
  m_cube->AddChild(MODEL_DIR + "2kNeptune.jpg", 4500, .18, 1.5, 3.9);
  m_cube->AddChild(MODEL_DIR + "2kPluto.jpg", 6000, 1, .15, .3);

  // Set up the shaders
  m_shader = new Shader();
  if(!m_shader->Initialize())
  {
    printf("Shader Failed to Initialize\n");
    return false;
  }

  // Add the vertex shader
  if(!m_shader->AddShader(GL_VERTEX_SHADER))
  {
    printf("Vertex Shader failed to Initialize\n");
    return false;
  }

  // Add the fragment shader
  if(!m_shader->AddShader(GL_FRAGMENT_SHADER))
  {
    printf("Fragment Shader failed to Initialize\n");
    return false;
  }

  // Connect the program
  if(!m_shader->Finalize())
  {
    printf("Program to Finalize\n");
    return false;
  }

  // Locate the projection matrix in the shader
  m_projectionMatrix = m_shader->GetUniformLocation("projectionMatrix");
  if (m_projectionMatrix == INVALID_UNIFORM_LOCATION)
  {
    printf("m_projectionMatrix not found\n");
    return false;
  }

  // Locate the view matrix in the shader
  m_viewMatrix = m_shader->GetUniformLocation("viewMatrix");
  if (m_viewMatrix == INVALID_UNIFORM_LOCATION)
  {
    printf("m_viewMatrix not found\n");
    return false;
  }

  // Locate the model matrix in the shader
  m_modelMatrix = m_shader->GetUniformLocation("modelMatrix");
  if (m_modelMatrix == INVALID_UNIFORM_LOCATION)
  {
    printf("m_modelMatrix not found\n");
    return false;
  }

  //enable depth testing
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  return true;
}

void Graphics::Update(unsigned int dt)
{
  // Update the object
  m_cube->Update(dt,glm::mat4(1.0), timeScale, orbitScale);
}

void Graphics::Render()
{
  //clear the screen
  glClearColor(0.0, 0.0, 0.2, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Start the correct program
  m_shader->Enable();

  // Send in the projection and view to the shader
  glUniformMatrix4fv(m_projectionMatrix, 1, GL_FALSE, glm::value_ptr(m_camera->GetProjection()));
  glUniformMatrix4fv(m_viewMatrix, 1, GL_FALSE, glm::value_ptr(m_camera->GetView()));

  // Render the object
  //glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_cube->GetModel()));
  m_cube->Render(m_modelMatrix);

  // Get any errors from OpenGL
  auto error = glGetError();
  if ( error != GL_NO_ERROR )
  {
    std::string val = ErrorString( error );
    std::cout<< "Error initializing OpenGL! " << error << ", " << val << std::endl;
  }
}

std::string Graphics::ErrorString(GLenum error)
{
  if(error == GL_INVALID_ENUM)
  {
    return "GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument.";
  }

  else if(error == GL_INVALID_VALUE)
  {
    return "GL_INVALID_VALUE: A numeric argument is out of range.";
  }

  else if(error == GL_INVALID_OPERATION)
  {
    return "GL_INVALID_OPERATION: The specified operation is not allowed in the current state.";
  }

  else if(error == GL_INVALID_FRAMEBUFFER_OPERATION)
  {
    return "GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete.";
  }

  else if(error == GL_OUT_OF_MEMORY)
  {
    return "GL_OUT_OF_MEMORY: There is not enough memory left to execute the command.";
  }
  else
  {
    return "None";
  }
}

Object* Graphics::GetCube() const
{
  return m_cube;
}

