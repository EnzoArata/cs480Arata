#include "object.h"
#include "graphics.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#define GLM_ENABLE_EXPERIMENTAL

using namespace std;


Object *objectReference;

Object::Object(Engine *engineReference)
{
    engineRef = engineReference;
    parentRef = NULL;



  Vertices = {
    {{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}},
    {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
    {{-1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
    {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}},
    {{1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 0.0f}},
    {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}},
    {{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f}},
    {{-1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}}
  };

  Indices = {
    2, 3, 4,
    8, 7, 6,
    1, 5, 6,
    2, 6, 7,
    7, 8, 4,
    1, 4, 8,
    1, 2, 4,
    5, 8, 6,
    2, 1, 6,
    3, 2, 7,
    3, 7, 4,
    5, 1, 8
  };

  // The index works at a 0th index
  for(unsigned int i = 0; i < Indices.size(); i++)
  {
    Indices[i] = Indices[i] - 1;
  }

  angle = 0.0f;

  glGenBuffers(1, &VB);
  glBindBuffer(GL_ARRAY_BUFFER, VB);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

  glGenBuffers(1, &IB);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * Indices.size(), &Indices[0], GL_STATIC_DRAW);

  rotationRate = 1.0f;
  rotationRate2 = 1.0f;
  angleMultiplier = 1.0f;
  angle2Multiplier = -1.0f;
  scaleValue = glm::vec3(1.0, 1.0, 1.0);
}

Object::Object(Engine *engineReference, string fileIn)
{
    engineRef = engineReference;
    parentRef = NULL;
    std::string t_input = " ";

    cout << "Please enter a valid .obg file name:\n>";
    getline(cin, t_input);
    cout << "Attempting to Load: " << t_input << endl << endl;
    std::string::size_type t_sizetype;
    std::string tempFileName = "Objects/" + t_input;
    std::string s;
    std::fstream inFile;
    Vertex t_Vertex = {{0, 0, 0}, {0, 0, 0}};
    char t_char;
    bool shorten = true;
    float t_float;
    float t_color;
    int t_int;
    int counter = 0;
    char badChar = '/';
    inFile.open(tempFileName);
    while(!inFile.eof())
    {
        inFile.get(t_char);
        if(inFile.eof())
        {
            inFile.close();
            break;
        }
        if(t_char == 'v')
        {
            inFile.get(t_char);
            if(t_char == 'n')
            {

            }
            else
            {
                std::cout << "Found 'v' " << std::endl;
                getline(inFile, s, ' ');
                t_float = std::stof(s, &t_sizetype );
                t_Vertex.vertex.x = t_float;
                std::cout << t_float << "  ";
                getline(inFile, s, ' ');
                t_float = std::stof(s, &t_sizetype );
                t_Vertex.vertex.y = t_float;
                std::cout << t_float << "  ";
                getline(inFile, s);
                t_float = std::stof(s, &t_sizetype );
                t_Vertex.vertex.z = t_float;
                std::cout << t_float << endl;
                t_color = (rand() / (float)RAND_MAX * 1) ;
                t_Vertex.color.x = (t_color);
                t_color = (rand() / (float)RAND_MAX * 1) ;
                t_Vertex.color.y = (t_color);
                t_color = (rand() / (float)RAND_MAX * 1) ;
                t_Vertex.color.z = (t_color);
                Vertices.push_back(t_Vertex);
            }




        }
        else if(t_char == 'f')
        {
            inFile.get(t_char);
            std::cout << "Found 'f' " << std::endl;
            for(int i=0;i<2;i++)
            {
                getline(inFile, s, ' ');

                for(int i = 0;i<s.size();i++)
                {

                    if(s.at(i) == '/')
                    {

                        break;
                    }
                    else
                    {
                        counter++;
                    }
                }
                s.resize(counter);
                counter = 0;

                t_int = std::stoi(s, &t_sizetype);
                std::cout << t_int << ", ";


                Indices.push_back(t_int);

            }
            getline(inFile, s);
            for(int i = 0;i<s.size();i++)
                {

                    if(s.at(i) == '/')
                    {

                        break;
                    }
                    else
                    {
                        counter++;
                    }
                }
                s.resize(counter);
                counter = 0;
            t_int = std::stof(s, &t_sizetype);
            Indices.push_back(t_int);
            std::cout << t_int << std::endl;








        }
        else
        {
            getline(inFile, s);
        }


    }



  /*Vertices = {
    {{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}},
    {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
    {{-1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
    {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}},
    {{1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 0.0f}},
    {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}},
    {{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f}},
    {{-1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}}
  };

  Indices = {
    2, 3, 4,
    8, 7, 6,
    1, 5, 6,
    2, 6, 7,
    7, 8, 4,
    1, 4, 8,
    1, 2, 4,
    5, 8, 6,
    2, 1, 6,
    3, 2, 7,
    3, 7, 4,
    5, 1, 8
  };*/

  // The index works at a 0th index
  for(unsigned int i = 0; i < Indices.size(); i++)
  {
    Indices[i] = Indices[i] - 1;
  }

  angle = 0.0f;

  glGenBuffers(1, &VB);
  glBindBuffer(GL_ARRAY_BUFFER, VB);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

  glGenBuffers(1, &IB);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * Indices.size(), &Indices[0], GL_STATIC_DRAW);

  rotationRate = 1.0f;
  rotationRate2 = 1.0f;
  angleMultiplier = 1.0f;
  angle2Multiplier = -1.0f;
  scaleValue = glm::vec3(1.0, 1.0, 1.0);
}

Object::Object(Engine *engineReference, Object *parentReference)
{
    engineRef = engineReference;


  Vertices = {
    {{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 0.0f}},
    {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
    {{-1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
    {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}},
    {{1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 0.0f}},
    {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}},
    {{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f}},
    {{-1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}}
  };

  Indices = {
    2, 3, 4,
    8, 7, 6,
    1, 5, 6,
    2, 6, 7,
    7, 8, 4,
    1, 4, 8,
    1, 2, 4,
    5, 8, 6,
    2, 1, 6,
    3, 2, 7,
    3, 7, 4,
    5, 1, 8
  };

  // The index works at a 0th index
  for(unsigned int i = 0; i < Indices.size(); i++)
  {
    Indices[i] = Indices[i] - 1;
  }

  angle = 0.0f;

  glGenBuffers(1, &VB);
  glBindBuffer(GL_ARRAY_BUFFER, VB);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

  glGenBuffers(1, &IB);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * Indices.size(), &Indices[0], GL_STATIC_DRAW);

  rotationRate = 1.0f;
  rotationRate2 = 1.0f;
  scaleValue = glm::vec3(1.0, 1.0, 1.0);
  angleMultiplier = 1.0f;
  angle2Multiplier = -1.0f;

  parentRef = parentReference;
  parentRef->addChild(this);
}

Object::~Object()
{
  Vertices.clear();
  Indices.clear();
}

void Object::Update(unsigned int dt, glm::mat4 parentMatrix)
{
    if(parentRef == NULL)
    {
        if(engineRef->rotationDirection == false)
        {
            angleMultiplier = -1.0;
        }
        if(engineRef->rotationDirection == true)
        {
            angleMultiplier = 1.0;
        }
        if(engineRef->rotationMovement == false)
        {
            angle2Multiplier =  -1.0;
        }
        if(engineRef->rotationMovement == true)
        {
            angle2Multiplier = 1.0;
        }
    }
    else
    {
        if(engineRef->rotationDirection2 == false)
        {
            angleMultiplier = -1.0;
        }
        if(engineRef->rotationDirection2 == true)
        {
            angleMultiplier = 1.0;
        }
        if(engineRef->rotationMovement2 == false)
        {
            angle2Multiplier =  -1.0;
        }
        if(engineRef->rotationMovement2 == true)
        {
            angle2Multiplier = 1.0;
        }
    }



    angle += angleMultiplier * rotationRate * dt * M_PI/1000;
    angle2 += angle2Multiplier * rotationRate2 * dt * M_PI/1000;


    model =parentMatrix * glm::translate( glm::mat4(1.0f), glm::vec3( 3.0f * cos( angle2 ), 0.0f, 3.0f* sin( angle2) ) );

    glm::mat4 modelRef = model;

    for(int i=0; i<childrenRef.size();i++)
    {

        childrenRef[i]->Update(dt, modelRef);

    }

    model *=   glm::rotate( model, (angle), glm::vec3(0.0, 1.0, 0.0))
             * glm::scale( scaleValue );










}

glm::mat4 Object::GetModel()
{
  return model;
}

void Object::Render()
{
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, VB);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,color));

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);

  glDrawElements(GL_TRIANGLES, Indices.size(), GL_UNSIGNED_INT, 0);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
}


void Object::addChild(Object *childReference)
{

    childrenRef.push_back (childReference);

}

void Object::setScale(glm::vec3 newSize)
{

    scaleValue = newSize;

}
