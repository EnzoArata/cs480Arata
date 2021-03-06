#include "graphics.h"
#include <math.h>

Graphics::Graphics()
{
  BulletInit();
  switcher = false;
  scoreFlag = false;
  moveLeftFlag = false;
  moveRightFlag = false;
  jumpFlag = false;
  dropBarrelFlag = false;
  lives = 3;
  gameScore = 0;
  timeBtwJump = 1000.0;
  timeSinceJump = 0.0;
  timeBtwSpawns = 2500.0;
  timeSinceSpawn = 0.0;
  timeBtwDrop = 1000.0;
  timeSinceDrop = 0.0;
  timeBtwScoring = 400.0;
  ladderCD = 800;
  despawnHeight =-25;
  ambIntensity = 0.0;
  lightHeight = 10;
  SHADOW_WIDTH = 1024;
  SHADOW_HEIGHT = 1024;
  movingLeft = true;
  spawnlocation = btVector3(2, 20, -.5);
  spawnlocation = btVector3(0, 0, 0);
  playerOnLadder = false;

}

Graphics::~Graphics()
{
  delete m_camera;
  delete m_shader;
  m_camera = NULL;
  m_shader = NULL;

  for (Object *obj : Objects)
    dynamicsWorld->removeRigidBody(obj->GetRigidBody());

  for (barrel *bar : barrels)
    dynamicsWorld->removeRigidBody(bar->object->GetRigidBody());

  for (ladder *lad : ladders)
    dynamicsWorld->removeRigidBody(lad->object->GetRigidBody());

  delete dynamicsWorld;
  dynamicsWorld = NULL;

  for (Object *obj : Objects)
    delete obj;

  for (barrel *bar : barrels)
    delete bar;

  for (ladder *lad : ladders)
    delete lad;

  Objects.clear();
  barrels.clear();
  ladders.clear();
}

bool Graphics::Initialize(int width, int height, int Level)
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

  // Create all objects
  CreateObjects(Level);

  // Set up the shaders
  m_shader = new Shader();
  otherShader = new Shader();
  shadow_shader = new Shader();
  if(!m_shader->Initialize())
  {
    printf("Shader Failed to Initialize\n");
    return false;
  }

  // Add the vertex shader
  if(!m_shader->AddShader(GL_VERTEX_SHADER, "../shaders/vert_shader_phong"))
  {
    printf("Vertex Shader failed to Initialize\n");
    return false;
  }

  // Add the fragment shader
  if(!m_shader->AddShader(GL_FRAGMENT_SHADER, "../shaders/frag_shader_phong"))
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

  m_lightSpaceMatrix = m_shader->GetUniformLocation("lightSpaceMatrix");
  if (m_lightSpaceMatrix == INVALID_UNIFORM_LOCATION)
  {
    printf("m_lightSpaceMatrix not found\n");
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

  if(!otherShader->Initialize())
  {
    printf("Shader Failed to Initialize\n");
    return false;
  }

  // Add the vertex shader
  if(!otherShader->AddShader(GL_VERTEX_SHADER, "../shaders/vert_shader_gourand"))
  {
    printf("Vertex Shader failed to Initialize\n");
    return false;
  }

  // Add the fragment shader
  if(!otherShader->AddShader(GL_FRAGMENT_SHADER, "../shaders/frag_shader_gourand"))
  {
    printf("Fragment Shader failed to Initialize\n");
    return false;
  }

  // Connect the program
  if(!otherShader->Finalize())
  {
    printf("Program to Finalize\n");
    return false;
  }

  // Locate the projection matrix in the shader
  other_projectionMatrix = otherShader->GetUniformLocation("projectionMatrix");
  if (other_projectionMatrix == INVALID_UNIFORM_LOCATION)
  {
    printf("other_projectionMatrix not found\n");
    return false;
  }

  // Locate the view matrix in the shader
  other_viewMatrix = otherShader->GetUniformLocation("viewMatrix");
  if (other_viewMatrix == INVALID_UNIFORM_LOCATION)
  {
    printf("other_viewMatrix not found\n");
    return false;
  }

  // Locate the model matrix in the shader
  other_modelMatrix = otherShader->GetUniformLocation("modelMatrix");
  if (other_modelMatrix == INVALID_UNIFORM_LOCATION)
  {
    printf("other_modelMatrix not found\n");
    return false;
  }


  //shadow mapping
  if(!shadow_shader->Initialize())
  {
    printf("Shader Failed to Initialize\n");
    return false;
  }

  // Add the vertex shader
  if(!shadow_shader->AddShader(GL_VERTEX_SHADER, "../shaders/shadow_vert"))
  {
    printf("Vertex Shader failed to Initialize\n");
    return false;
  }

  // Add the fragment shader
  if(!shadow_shader->AddShader(GL_FRAGMENT_SHADER, "../shaders/shadow_frag"))
  {
    printf("Fragment Shader failed to Initialize\n");
    return false;
  }

  // Connect the program
  if(!shadow_shader->Finalize())
  {
    printf("Program to Finalize\n");
    return false;
  }

  //GLuint texLoc = glGetUniformLocation(m_shader->m_shaderProg, "sampler");
  //glUniform1i(texLoc, 0);
  //texLoc = glGetUniformLocation(m_shader->m_shaderProg, "depthMap");
  //glUniform1i(texLoc, 1);




  //enable depth testing
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  /*
  // creating the lightspace matrix
  float near_plane = 1.0f, far_plane = 7.5f;
  glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);


  glm::mat4 lightView = glm::lookAt(glm::vec3(0, 0.0, -20.0),
                                    glm::vec3( 0.0f, 0.0f,  0.0f),
                                    glm::vec3( 0.0f, 1.0f,  0.0f));

  lightSpaceMatrix = lightProjection * lightView;

    glGenFramebuffers(1, &depthMapFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

  glGenTextures(1, &depthMap);
  glBindTexture(GL_TEXTURE_2D, depthMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
             SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);

  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  GLuint texLoc = glGetUniformLocation(m_shader->m_shaderProg, "depthMap");
  glUniform1i(texLoc, 1);*/

  return true;
}

void Graphics::Update(unsigned int dt)
{
  float lavaSpeed = 0.01;
  float playerY = player->GetRigidBody()->getCenterOfMassTransform().getOrigin().y();
  btTransform newTrans;

  floor->GetRigidBody()->getMotionState()->getWorldTransform(newTrans);
  if (playerY + despawnHeight > newTrans.getOrigin().y())
    newTrans.getOrigin() += (btVector3(0, lavaSpeed*3, 0));
  else
    newTrans.getOrigin() += (btVector3(0, lavaSpeed, 0));
  floor->GetRigidBody()->getMotionState()->setWorldTransform(newTrans);

  if (climbUpFlag){
    climbUp();
  }
  if (climbDownFlag){
    climbDown();
  }
  if (climbLeftFlag){
    climbLeft();
  }
  if (climbRightFlag){
    climbRight();
  }
  // Update the object

  m_camera->Update(glm::vec3(player->GetRigidBody()->getCenterOfMassTransform().getOrigin().x(),
                      		   playerY,
                    		     player->GetRigidBody()->getCenterOfMassTransform().getOrigin().z()),
                             floor->GetRigidBody()->getCenterOfMassTransform().getOrigin().y()+3.75); //3.75 is the offset of the lavaplane from its origin

  //flipperR->GetRigidBody()->applyTorque(btVector3(1,1,1));
  barrelSpawner(dt,playerY);

  for (auto &i : Objects) {
  	i->Update(dt);
  }

  for (auto it = Objects.begin(); it != Objects.end(); ) {
  	if ((*it)->destroy) {
  	  dynamicsWorld->removeRigidBody((*it)->GetRigidBody());
  	  Objects.erase(it);
  	} else {
  	  ++it;
  	}
  }

  for (auto &i : barrels) {
    if(i->object->GetRigidBody()->getCenterOfMassTransform().getOrigin().y() < floor->GetRigidBody()->getCenterOfMassTransform().getOrigin().y()+2){
      i->object->destroy = true;
    } else {
      i->object->Update(dt);
    }
  }

  for (auto it = barrels.begin(); it != barrels.end(); ) {
    if ((*it)->object->destroy) {
      dynamicsWorld->removeRigidBody((*it)->object->GetRigidBody());
      barrels.erase(it);
    } else {
      ++it;
    }
  }

  for (auto &i : ladders) {
    if(i->object->GetRigidBody()->getCenterOfMassTransform().getOrigin().y() < despawnHeight + player->GetRigidBody()->getCenterOfMassTransform().getOrigin().y()){
      //i->object->destroy = true;
    } else {
      i->object->Update(dt);
    }
  }

  for (auto it = ladders.begin(); it != ladders.end(); ) {
    if ((*it)->object->destroy) {
      dynamicsWorld->removeRigidBody((*it)->object->GetRigidBody());
      ladders.erase(it);
    } else {
      ++it;
    }
  }
  //if(playerOnLadder == false)
  //{
  	if(moveRightFlag == true)
    	moveRight();
  	if(moveLeftFlag == true)
    	moveLeft();
  //}
  

  jump(dt);
  checkBarrelDrop();
  checkLadderState(dt);
  checkPlayerOnLadder(dt);
  descendBarrel(dt);
  checkBarrelJumped(dt);

  dynamicsWorld->stepSimulation(dt, 5);
}

void Graphics::Render()
{
  //clear the screen
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
/*
  //shadow_shader->Enable();
  m_shader->Enable();



  // Render the objects




  glClear(GL_DEPTH_BUFFER_BIT);


  for (auto &i : Objects) {
    if (i->render)
      i->Render(m_modelMatrix, m_shader);
  }
  for (auto &i : barrels) {
    if (i->object->render)
      i->object->Render(m_modelMatrix, m_shader);
  }
  for (auto &i : ladders) {
    if (i->object->render)
      i->object->Render(m_modelMatrix, m_shader);
  }
    //glBindTexture(GL_TEXTURE_2D, depthMap);

  GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

  if (Status != GL_FRAMEBUFFER_COMPLETE) {
      printf("FB error, status: 0x%x\n", Status);
      exit(0);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glViewport(0, 0, 1080, 920);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);*/

  if (!switcher)
  {
    // Start the correct program
    m_shader->Enable();



    // Send in the projection and view to the shader
    GLint temp = m_shader->GetUniformLocation("eyePos");
    glUniform3f(temp, m_camera->eyePos.x, m_camera->eyePos.y, m_camera->eyePos.z);

    temp = m_shader->GetUniformLocation("playerPos");
    glUniform3f(temp, (float) player->GetRigidBody()->getCenterOfMassTransform().getOrigin().x(),
                      (float) player->GetRigidBody()->getCenterOfMassTransform().getOrigin().y(),
                      (float) player->GetRigidBody()->getCenterOfMassTransform().getOrigin().z());

    temp = m_shader->GetUniformLocation("AmbientProduct");
	glUniform4f(temp, ambIntensity, ambIntensity, ambIntensity, 1);

	temp = m_shader->GetUniformLocation("LightPosition");
	glUniform3f(temp, 0, lightHeight+player->GetRigidBody()->getCenterOfMassTransform().getOrigin().y(), -lightHeight*2);

	temp = m_shader->GetUniformLocation("coneDirection");
	glUniform3f(temp, 0, -(lightHeight+player->GetRigidBody()->getCenterOfMassTransform().getOrigin().y()), lightHeight*2);

	temp = m_shader->GetUniformLocation("coneCutOff");
	glUniform1f(temp, .995);

	temp = m_shader->GetUniformLocation("spotExponent");
	glUniform1f(temp, .1);

    glUniformMatrix4fv(m_projectionMatrix, 1, GL_FALSE, glm::value_ptr(m_camera->GetProjection()));
    glUniformMatrix4fv(m_projectionMatrix, 1, GL_FALSE, glm::value_ptr(m_camera->GetProjection()));
    glUniformMatrix4fv(m_viewMatrix, 1, GL_FALSE, glm::value_ptr(m_camera->GetView()));

    // Render the object
    for (auto &i : Objects) {
      if (i->render)
        //i->LoadShadowMap(depthMap, 1);
        i->Render(m_modelMatrix, m_shader);
    }
    for (auto &i : barrels) {
      if (i->object->render)
        //i->object->LoadShadowMap(depthMap, 1);
        i->object->Render(m_modelMatrix, m_shader);
    }
    for (auto &i : ladders) {
      if (i->object->render)
        //i->object->LoadShadowMap(depthMap, 1);
        i->object->Render(m_modelMatrix, m_shader);
    }
  }
  else
  {
    otherShader->Enable();

    // Send in the projection and view to the shader
    GLint temp = otherShader->GetUniformLocation("eyePos");
    glUniform3f(temp, m_camera->eyePos.x, m_camera->eyePos.y, m_camera->eyePos.z);

    temp = otherShader->GetUniformLocation("playerPos");
    glUniform3f(temp, (float) player->GetRigidBody()->getCenterOfMassTransform().getOrigin().x(),
                      (float) player->GetRigidBody()->getCenterOfMassTransform().getOrigin().y(),
                      (float) player->GetRigidBody()->getCenterOfMassTransform().getOrigin().z());

    temp = otherShader->GetUniformLocation("AmbientProduct");
	glUniform4f(temp, ambIntensity, ambIntensity, ambIntensity, 1);

	temp = otherShader->GetUniformLocation("LightPosition");
	glUniform3f(temp, 0, lightHeight, 0);

	temp = otherShader->GetUniformLocation("coneDirection");
	glUniform3f(temp, 0, -lightHeight, 0);

	temp = otherShader->GetUniformLocation("coneCutOff");
	glUniform1f(temp, .995);

	temp = otherShader->GetUniformLocation("spotExponent");
	glUniform1f(temp, .1);

    glUniformMatrix4fv(other_projectionMatrix, 1, GL_FALSE, glm::value_ptr(m_camera->GetProjection()));
    glUniformMatrix4fv(other_viewMatrix, 1, GL_FALSE, glm::value_ptr(m_camera->GetView()));

    // Render the object
    for (auto &i : Objects) {
      if (i->render)
        i->Render(other_modelMatrix, otherShader);
    }
    for (auto &i : barrels) {
      if (i->object->render)
        i->object->Render(other_modelMatrix, otherShader);
    }
    for (auto &i : ladders) {
      if (i->object->render)
        i->object->Render(other_modelMatrix, otherShader);
    }
  }

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
    return "GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument.";

  else if(error == GL_INVALID_VALUE)
    return "GL_INVALID_VALUE: A numeric argument is out of range.";

  else if(error == GL_INVALID_OPERATION)
    return "GL_INVALID_OPERATION: The specified operation is not allowed in the current state.";

  else if(error == GL_INVALID_FRAMEBUFFER_OPERATION)
    return "GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete.";

  else if(error == GL_OUT_OF_MEMORY)
    return "GL_OUT_OF_MEMORY: There is not enough memory left to execute the command.";

  else
    return "None";
}

void Graphics::loadLevel0(){
  bottom = -50;

  playerSpawn = btVector3(10, 0, 0);
  platformSpawner(4, glm::vec3(20,35,0), -30);
  platformSpawner(4, glm::vec3(-20,25,0), 30);
  platformSpawner(4, glm::vec3(20,10,0), -15);
  platformSpawner(1, glm::vec3(-26,2,0), 0);
  platformSpawner(4, glm::vec3(-20,0,0), 15);
  platformSpawner(4, glm::vec3(20,-10,0), -15);
  platformSpawner(4, glm::vec3(-20,-20,0), 15);
  platformSpawner(4, glm::vec3(20,-25,0), -30);
  platformSpawner(4, glm::vec3(-20,-40,0), 15);
  platformSpawner(4, glm::vec3(10,bottom,0), 0);
}
void Graphics::loadLevel1(){

  Object* tempObject;
  bottom = -50;
  playerSpawn = btVector3(10, 0, 0);
  platformSpawner(1, glm::vec3(31,61,0), 0);
  platformSpawner(4, glm::vec3(20,50,0), -30);
  platformSpawner(3, glm::vec3(-5,30,0), 30);
  platformSpawner(1, glm::vec3(23,19,0), -0);
  platformSpawner(2, glm::vec3(15,15,0), -30);
  platformSpawner(1, glm::vec3(0,5,0), -0);
  platformSpawner(6, glm::vec3(12,-5,0), -30);
  platformSpawner(1, glm::vec3(-34,-26,0), 0);
  platformSpawner(1, glm::vec3(-25 ,-33,0), 0);
  platformSpawner(3, glm::vec3(-15,-40,0), 15);
  platformSpawner(4, glm::vec3(10,bottom,0), 0);
  spawnlocation = btVector3(2, 20, -.5);
  tempObject = new Object("Ladder.obj", "bluebaby.jpg", 0,0, btVector3(0, 6.5, -.5) );
  ladder *tempLadder = new ladder();
  *tempLadder = {tempObject, 0, false};
  tempObject->GetRigidBody()->setCollisionFlags(tempLadder->object->GetRigidBody()->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
  tempObject->GetRigidBody()->setActivationState(DISABLE_DEACTIVATION);
  ladders.push_back(tempLadder);  

  tempObject = new Object("Ladder.obj", "bluebaby.jpg", 0,0, btVector3(28,62.5,-.5) );
  tempLadder = new ladder();
  *tempLadder = {tempObject, 0, false};
  tempObject->GetRigidBody()->setCollisionFlags(tempLadder->object->GetRigidBody()->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
  tempObject->GetRigidBody()->setActivationState(DISABLE_DEACTIVATION);
  ladders.push_back(tempLadder); 

  tempObject = new Object("Ladder.obj", "bluebaby.jpg", 0,0, btVector3(28,55,-.5) );
  tempLadder = new ladder();
  *tempLadder = {tempObject, 0, false};
  tempObject->GetRigidBody()->setCollisionFlags(tempLadder->object->GetRigidBody()->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
  tempObject->GetRigidBody()->setActivationState(DISABLE_DEACTIVATION);
  ladders.push_back(tempLadder);  

  tempObject = new Object("DK_Arm_UP.obj", "donkey_tex.png", 0,0, btVector3(33,63,0));
  //*tempLadder = {tempObject, 0, false};
  tempObject->GetRigidBody()->setCollisionFlags(tempObject->GetRigidBody()->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
  tempObject->GetRigidBody()->setActivationState(DISABLE_DEACTIVATION);
  Objects.push_back(tempObject);

}
void Graphics::loadLevel2(){

}

void Graphics::CreateObjects(int Level)
{
  std::cout << "Level loaded: " << Level << std::endl;
  switch (Level){
    case 1: loadLevel1();
    break;
    case 2: loadLevel2();
    break;
    default:
      loadLevel0();
  }



  Object* tempObject;
  myBarrel = new Object("Barrel2.obj", "DKBarrel.png", 0,0, btVector3(2, 20, -50));
  std::cout << "spawning dk" << std::endl;
  

  floor = tempObject = new Object("Floor.obj", "2kSun.jpg", 0,0, btVector3(0,bottom-5-3.75,0));
  tempObject->GetRigidBody()->setCollisionFlags(tempObject->GetRigidBody()->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
  tempObject->GetRigidBody()->setActivationState(DISABLE_DEACTIVATION);
  Objects.push_back(tempObject);

  


  /*tempObject = new Object("Ladder.obj", "bluebaby.jpg", 5,1, btVector3(8, 9, .5));
  ladder *tempLadder = new ladder();
  *tempLadder = {tempObject, 0, false};
  tempObject->GetRigidBody()->setCollisionFlags(tempLadder->object->GetRigidBody()->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
  tempObject->GetRigidBody()->setActivationState(DISABLE_DEACTIVATION);
  ladders.push_back(tempLadder);

  tempObject = new Object("Ladder.obj", "bluebaby.jpg", 5,1, btVector3(-4, 5, .5));
  tempLadder = new ladder();
  *tempLadder = {tempObject, 0, false};
  tempObject->GetRigidBody()->setCollisionFlags(tempLadder->object->GetRigidBody()->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
  tempObject->GetRigidBody()->setActivationState(DISABLE_DEACTIVATION);
  ladders.push_back(tempLadder);*/


  player = tempObject = new Object("PlayerSprite.obj", "marioL.png", 1,5, btVector3(playerSpawn.x(),bottom+2,0));
  tempObject->GetRigidBody()->setActivationState(DISABLE_DEACTIVATION);
  tempObject->GetRigidBody()->setAngularFactor(btVector3(0,0,0));
  Objects.push_back(tempObject);

  for (auto &i : Objects) {
    dynamicsWorld->addRigidBody(i->GetRigidBody());
  }
  for (auto &i : barrels) {
    dynamicsWorld->addRigidBody(i->object->GetRigidBody());
  }
  for (auto &i : ladders) {
    dynamicsWorld->addRigidBody(i->object->GetRigidBody());
  }
}

void Graphics::BulletInit()
{
	btBroadphaseInterface *broadphase =
		new btDbvtBroadphase();

	btDefaultCollisionConfiguration *collisionConfiguration =
		new btDefaultCollisionConfiguration();

	btCollisionDispatcher *dispatcher =
		new btCollisionDispatcher(collisionConfiguration);

	btSequentialImpulseConstraintSolver *solver =
		new btSequentialImpulseConstraintSolver();

	dynamicsWorld =
		new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);

	dynamicsWorld->setGravity(btVector3(0, -3, 0));
}

void Graphics::platformSpawner(int platforms, glm::vec3 origin, int angle){
	Object* tempObject;
	glm::vec3 platformOffset;
	std::string modelName;

	switch(angle) {
    	case 15 :
 	   		platformOffset = glm::vec3( 8*cos((15*M_PI)/180), -(8*sin((15*M_PI)/180)), 0);
			modelName = "15DegPlatform.obj";
            break;

 	   case -15 :
 	   		platformOffset = glm::vec3(-(8*cos((15*M_PI)/180)), -(8*sin((15*M_PI)/180)), 0);
			modelName = "-15DegPlatform.obj";
            break;

    	case 30 :
			platformOffset = glm::vec3( 8*cos((30*M_PI)/180), -(8*sin((30*M_PI)/180)), 0);
			modelName = "30DegPlatform.obj";
            break;
 	   case -30 :
 	   		platformOffset = glm::vec3( -(8*cos((30*M_PI)/180)),-(8*sin((30*M_PI)/180)), 0);
			modelName = "-30DegPlatform.obj";
            break;
        default :
        	platformOffset = glm::vec3(8,0,0);
			modelName = "0DegPlatform.obj";
        	break;
	}


	for (int i = 0; i<platforms; i++){
		btVector3 pos = btVector3(origin.x+platformOffset.x*i, origin.y+platformOffset.y*i, origin.z+platformOffset.z*i);
		tempObject = new Object(modelName, "reddy.jpg", 0,0, pos);
		Objects.push_back(tempObject);
    if (rand()%3 == 0){
      tempObject = new Object("Ladder.obj", "bluebaby.jpg", 0,0, btVector3(pos.x(), pos.y()+1.5, -.5));
      ladder *tempLadder = new ladder();
      *tempLadder = {tempObject, 0, false};
      tempObject->GetRigidBody()->setCollisionFlags(tempLadder->object->GetRigidBody()->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
      tempObject->GetRigidBody()->setActivationState(DISABLE_DEACTIVATION);
      ladders.push_back(tempLadder);    
    }

	}
	//tempObject->GetRigidBody()->setRestitution(1.0);
}

void Graphics::barrelSpawner(unsigned int dt, float playerHeight){
	timeSinceSpawn += dt;

	if (timeBtwSpawns<timeSinceSpawn){
    if (spawnlocation.y()-25 < playerHeight){
      spawnlocation += btVector3(0,25,0);
    } else if (spawnlocation.y()-30 > playerHeight){
      spawnlocation += btVector3(0,-10,0);
    }
		timeSinceSpawn = 0;
    int spawnAmount = rand()%2+1;
    for (int i=0;i<spawnAmount;i++)
		  spawnBarrel(spawnlocation);
	}
}

void Graphics::spawnBarrel(btVector3 pos)
{

  Object* tempObject = new Object(*myBarrel, pos);//btVector3(2, 20, -.5));//Object("Barrel.obj", "rednice.jpg", 5,1, btVector3(2, 20, 0));
  barrel *tempBarrel = new barrel();
  *tempBarrel = {tempObject, 0, false};
  tempObject->GetRigidBody()->setActivationState(DISABLE_DEACTIVATION);
  barrels.push_back(tempBarrel);
  dynamicsWorld->addRigidBody(tempBarrel->object->GetRigidBody());

}

void Graphics::SwitchShader()
{
  switcher ? switcher = false : switcher = true;
}

bool Graphics::HasDied()
{
  if (player->GetRigidBody()->getCenterOfMassTransform().getOrigin().y() < floor->GetRigidBody()->getCenterOfMassTransform().getOrigin().y()+4){
    btTransform newTrans;

    floor->GetRigidBody()->getMotionState()->getWorldTransform(newTrans);
    newTrans.setOrigin(btVector3(0, bottom-5-3.75, 0));
    floor->GetRigidBody()->getMotionState()->setWorldTransform(newTrans);
    return true;
  }
  return false;
}

void Graphics::ResetPlayer()
{
  //delete player;
  dynamicsWorld->removeRigidBody(player->GetRigidBody());
  Object* tempObject;
  player = tempObject = new Object("PlayerSprite.obj", "marioL.png", 1,5, btVector3(playerSpawn.x(), bottom+2, 0));
  tempObject->GetRigidBody()->setActivationState(DISABLE_DEACTIVATION);
  tempObject->GetRigidBody()->setAngularFactor(btVector3(0,0,0));
  dynamicsWorld->addRigidBody(tempObject->GetRigidBody());
  Objects.push_back(tempObject);
}

void Graphics::moveLeft()
{
    if (!movingLeft){
      player->LoadTexFile(ASSET_DIR + "marioL.png", 0);
      movingLeft=true;
    }
    btVector3 tempbtVec3 =  player->GetRigidBody()->getLinearVelocity();
    //if(tempbtVec3.x() < 0)
       //tempbtVec3 = btVector3(0, 0, 0);
    tempbtVec3 = tempbtVec3 + btVector3(.75, 0, 0);
    if(tempbtVec3.x() > 1)
      tempbtVec3 = btVector3(1, tempbtVec3.y(), 0);
    player->GetRigidBody()->setLinearVelocity(tempbtVec3);
}
void Graphics::moveRight()
{
    if (movingLeft){
      player->LoadTexFile(ASSET_DIR + "marioR.png", 0);
      movingLeft=false;
    }
    btVector3 tempbtVec3 =  player->GetRigidBody()->getLinearVelocity();
    //if(tempbtVec3.x() > 0)
       //tempbtVec3 = btVector3(0, 0, 0);
    tempbtVec3 = tempbtVec3 + btVector3(-.75, 0, 0);
    if(tempbtVec3.x() < -1)
      tempbtVec3 = btVector3(-1, tempbtVec3.y(), 0);
    /*if(tempbtVec3.y() < -2)
      tempbtVec3 = btVector3(tempbtVec3.x(), -2, 0);*/
    player->GetRigidBody()->setLinearVelocity(btVector3(tempbtVec3));
}

void Graphics::jump(unsigned int dt)
{
  timeSinceJump += dt;

  if (timeBtwJump<timeSinceJump && jumpFlag == true){
    timeSinceJump = 0;
    player->GetRigidBody()->applyCentralImpulse( btVector3( 0, 6.5, 0 ) );

  }
} 

//bool Graphics::Grounded()  


void Graphics::descendBarrel(unsigned int dt)
{

  btTransform newTrans;
  for(int j=0; j<barrels.size();j++)
  {
  	barrels[j]->dropTimer += dt;
  	if(barrels[j]->hasDropped == true)
  	{
	    barrels[j]->object->GetRigidBody()->getMotionState()->getWorldTransform(newTrans);
	    newTrans.getOrigin() += (btVector3(0, -.1, 0));
	    barrels[j]->object->GetRigidBody()->getMotionState()->setWorldTransform(newTrans);
	}
	if (timeBtwDrop<barrels[j]->dropTimer && barrels[j]->hasDropped == true)
	{
		barrels[j]->hasDropped = false;
	    barrels[j]->dropTimer = 0;
	    resetBarrel(barrels[j]);
	}

  }


}
void Graphics::dropBarrel(barrel* passedBarrel)
{
	//std::cout << "dropping barrell" << std::endl;
    passedBarrel->object->GetRigidBody()->setCollisionFlags(passedBarrel->object->GetRigidBody()->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    passedBarrel->dropTimer = 0;
    passedBarrel->hasDropped = true;

}
void Graphics::resetBarrel(barrel* passedBarrel)
{
	//std::cout << "reseting barrell" << std::endl;
    passedBarrel->object->GetRigidBody()->setCollisionFlags(passedBarrel->object->GetRigidBody()->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
    //passedBarrel->object->GetRigidBody()->setActivationState(DISABLE_DEACTIVATION);

}
void Graphics::checkBarrelDrop()
{
	for(int i=0; i<ladders.size();i++)
	{
		for(int j=0; j<barrels.size();j++)
		{
			if( (barrels[j]->object->GetRigidBody()->getCenterOfMassTransform().getOrigin().y() -
				ladders[i]->object->GetRigidBody()->getCenterOfMassTransform().getOrigin().y() <= 5) &&
				(barrels[j]->object->GetRigidBody()->getCenterOfMassTransform().getOrigin().y() -
				ladders[i]->object->GetRigidBody()->getCenterOfMassTransform().getOrigin().y() >= 0 ) )
				{
					if((barrels[j]->object->GetRigidBody()->getCenterOfMassTransform().getOrigin().x() -
					ladders[i]->object->GetRigidBody()->getCenterOfMassTransform().getOrigin().x() <= 0.5) &&
					(barrels[j]->object->GetRigidBody()->getCenterOfMassTransform().getOrigin().x() -
					ladders[i]->object->GetRigidBody()->getCenterOfMassTransform().getOrigin().x() >= -0.5 ))
					{
						if(ladders[i]->cooldownFlag == false)
						{
							ladders[i]->cooldownFlag = true;
              if (rand()%3)
							  dropBarrel(barrels[j]);
							ladders[i]->ladderCooldown = 0;
						}

					}

				}
		}
	}
}
void Graphics::checkLadderState(unsigned int dt)
{
	for(int j=0; j<ladders.size();j++)
	{
  		ladders[j]->ladderCooldown += dt;
  		if (ladderCD<ladders[j]->ladderCooldown && ladders[j]->cooldownFlag == true)
		{
		    ladders[j]->cooldownFlag = false;
		    ladders[j]->ladderCooldown = 0;
		}
  }
}
void Graphics::checkPlayerOnLadder(unsigned int dt)
{
	bool playerFound = false;
	for(int i=0; i<ladders.size();i++)
	{
		if( (player->GetRigidBody()->getCenterOfMassTransform().getOrigin().y() -
				ladders[i]->object->GetRigidBody()->getCenterOfMassTransform().getOrigin().y() <= 0) &&
				(player->GetRigidBody()->getCenterOfMassTransform().getOrigin().y() -
				ladders[i]->object->GetRigidBody()->getCenterOfMassTransform().getOrigin().y() >= -10 ) )
				{
					if((player->GetRigidBody()->getCenterOfMassTransform().getOrigin().x() -
					ladders[i]->object->GetRigidBody()->getCenterOfMassTransform().getOrigin().x() <= 0.7) &&
					(player->GetRigidBody()->getCenterOfMassTransform().getOrigin().x() -
					ladders[i]->object->GetRigidBody()->getCenterOfMassTransform().getOrigin().x() >= -0.7 ))
					{
						playerOnLadder = true;
						playerFound = true;
						player->GetRigidBody()->setCollisionFlags(player->GetRigidBody()->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
					}
				}
	}
	if(playerFound == false)
	{
		playerOnLadder = false;
		 player->GetRigidBody()->setCollisionFlags(player->GetRigidBody()->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
	}


}
void Graphics::climbUp()
{
	btTransform newTrans;
  if(playerOnLadder == true){
    player->GetRigidBody()->getMotionState()->getWorldTransform(newTrans);
    newTrans.getOrigin() += (btVector3(0, .1, 0));
    player->GetRigidBody()->getMotionState()->setWorldTransform(newTrans);
  }

}

void Graphics::climbLeft()
{
  if(playerOnLadder == true){
    btTransform newTrans;
    player->GetRigidBody()->getMotionState()->getWorldTransform(newTrans);
    newTrans.getOrigin() += (btVector3(.1, 0, 0));
    player->GetRigidBody()->getMotionState()->setWorldTransform(newTrans);
  }


}
void Graphics::climbRight()
{
  if(playerOnLadder == true){
    btTransform newTrans;
    player->GetRigidBody()->getMotionState()->getWorldTransform(newTrans);
    newTrans.getOrigin() += (btVector3(-.1, 0, 0));
    player->GetRigidBody()->getMotionState()->setWorldTransform(newTrans);
  }


}
void Graphics::climbDown()
{
  if(playerOnLadder == true){
    btTransform newTrans;
    player->GetRigidBody()->getMotionState()->getWorldTransform(newTrans);
    newTrans.getOrigin() += (btVector3(0, -.1, 0));
    player->GetRigidBody()->getMotionState()->setWorldTransform(newTrans);
  }
}


void Graphics::checkBarrelJumped(unsigned int dt)
{
	//std::cout << "Score ifunction!" << std::endl;
	scoreCooldown += dt;
	for(int i=0; i<barrels.size();i++)
	{
		if( (player->GetRigidBody()->getCenterOfMassTransform().getOrigin().y() -
				barrels[i]->object->GetRigidBody()->getCenterOfMassTransform().getOrigin().y() <= 5) &&
				(player->GetRigidBody()->getCenterOfMassTransform().getOrigin().y() -
				barrels[i]->object->GetRigidBody()->getCenterOfMassTransform().getOrigin().y() >= 1 ) )
				{
					if((player->GetRigidBody()->getCenterOfMassTransform().getOrigin().x() -
					barrels[i]->object->GetRigidBody()->getCenterOfMassTransform().getOrigin().x() <= 1) &&
					(player->GetRigidBody()->getCenterOfMassTransform().getOrigin().x() -
					barrels[i]->object->GetRigidBody()->getCenterOfMassTransform().getOrigin().x() >= -1 ))
					{
						if(timeBtwScoring<scoreCooldown && playerOnLadder == false)
						{
							std::cout << "Score increased!" << std::endl;
							scoreCooldown = 0;
							gameScore += 20;
						}
						
					}
				}
	}
}
float Graphics::GetObjectDistance(Object* obj1, Object* obj2)
{
  float x = obj1->GetRigidBody()->getCenterOfMassTransform().getOrigin().x()
            - obj2->GetRigidBody()->getCenterOfMassTransform().getOrigin().x();

  float z = obj1->GetRigidBody()->getCenterOfMassTransform().getOrigin().z()
            - obj2->GetRigidBody()->getCenterOfMassTransform().getOrigin().z();

  return sqrt(pow(x, 2.0) + pow(z, 2.0));
}

Object* Graphics::GetPlayer() const
{
  return player;
}

std::vector<Object*> Graphics::GetObjects() const
{
  return Objects;
}

Camera* Graphics::GetCamera() const
{
  return m_camera;
}

btDiscreteDynamicsWorld* Graphics::GetDynamicsWorld() const
{
  return dynamicsWorld;
}