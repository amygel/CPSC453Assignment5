// ==========================================================================
// Barebones OpenGL Core Profile Boilerplate
//    using the GLFW windowing system (http://www.glfw.org)
//
// Loosely based on
//  - Chris Wellons' example (https://github.com/skeeto/opengl-demo) and
//  - Camilla Berglund's example (http://www.glfw.org/docs/latest/quick.html)
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <iterator>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>
#include <ctime>
#include "camera.h"

// Specify that we want the OpenGL core profile before including GLFW headers
#ifdef _WIN32
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#else
#include <glad/glad.h>
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define _USE_MATH_DEFINES
#include <math.h>

using namespace glm;
using namespace std;

// --------------------------------------------------------------------------
// OpenGL utility and support function prototypes

void QueryGLVersion();
bool CheckGLErrors();

string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);

vec2 mousePos_;
Camera cam_;
bool mousePressed_ = false;

// --------------------------------------------------------------------------
// Functions to set up OpenGL shader programs for rendering

struct MyShader
{
   // OpenGL names for vertex and fragment shaders, shader program
   GLuint  vertex;
   GLuint  fragment;
   GLuint  program;

   // initialize shader and program names to zero (OpenGL reserved value)
   MyShader() : vertex(0), fragment(0), program(0)
   {}
};

// load, compile, and link shaders, returning true if successful
bool InitializeShaders(MyShader *shader)
{
   // load shader source from files
   string vertexSource = LoadSource("vertex.glsl");
   string fragmentSource = LoadSource("fragment.glsl");
   if (vertexSource.empty() || fragmentSource.empty()) return false;

   // compile shader source into shader objects
   shader->vertex = CompileShader(GL_VERTEX_SHADER, vertexSource);
   shader->fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

   // link shader program
   shader->program = LinkProgram(shader->vertex, shader->fragment);

   // check for OpenGL errors and return false if error occurred
   return !CheckGLErrors();
}

// deallocate shader-related objects
void DestroyShaders(MyShader *shader)
{
   // unbind any shader programs and destroy shader objects
   glUseProgram(0);
   glDeleteProgram(shader->program);
   glDeleteShader(shader->vertex);
   glDeleteShader(shader->fragment);
}

// --------------------------------------------------------------------------
// Functions to set up OpenGL buffers for storing textures

struct MyTexture
{
   GLuint textureID;
   GLuint target;
   int width;
   int height;

   // initialize object names to zero (OpenGL reserved value)
   MyTexture() : textureID(0), target(0), width(0), height(0)
   {}
};

bool InitializeTexture(MyTexture* texture, const char* filename, GLuint target = GL_TEXTURE_2D)
{
   int numComponents;
   stbi_set_flip_vertically_on_load(true);
   unsigned char *data = stbi_load(filename, &texture->width, &texture->height, &numComponents, 0);
   if (data != nullptr)
   {
      texture->target = target;
      glGenTextures(1, &texture->textureID);
      glBindTexture(texture->target, texture->textureID);
      GLuint format = GL_RGB;
      switch (numComponents)
      {
      case 4:
         format = GL_RGBA;
         break;
      case 3:
         format = GL_RGB;
         break;
      case 2:
         format = GL_RG;
         break;
      case 1:
         format = GL_RED;
         break;
      default:
         cout << "Invalid Texture Format" << endl;
         break;
      };
      glTexImage2D(texture->target, 0, format, texture->width, texture->height, 0, format, GL_UNSIGNED_BYTE, data);

      // Note: Only wrapping modes supported for GL_TEXTURE_RECTANGLE when defining
      // GL_TEXTURE_WRAP are GL_CLAMP_TO_EDGE or GL_CLAMP_TO_BORDER
      glTexParameteri(texture->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(texture->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(texture->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(texture->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      // Clean up
      glBindTexture(texture->target, 0);
      stbi_image_free(data);
      return !CheckGLErrors();
   }
   return true; //error
}

// deallocate texture-related objects
void DestroyTexture(MyTexture *texture)
{
   glBindTexture(texture->target, 0);
   glDeleteTextures(1, &texture->textureID);
}

// --------------------------------------------------------------------------
// Functions to set up OpenGL buffers for storing geometry data

struct MyGeometry
{
   // OpenGL names for array buffer objects, vertex array object
   GLuint  vertexBuffer;
   GLuint  textureBuffer;
   GLuint  elementBuffer;
   GLuint  normalBuffer;
   GLuint  vertexArray;
   GLsizei elementCount;

   // initialize object names to zero (OpenGL reserved value)
   MyGeometry() : vertexBuffer(0), elementBuffer(0), normalBuffer(0), vertexArray(0), elementCount(0)
   {}
};

void generateSphere(vector<vec3>& points, vector<vec3>& normals,
   vector<unsigned int>& indices, float r, int uDivisions, int vDivisions)
{
   float uStep = 360.0f / static_cast<float>(uDivisions);
   float vStep = 360.0f / static_cast<float>(vDivisions);

   float u = 0.f;
   float v = 0.f;
   vec3 center = vec3(0.0f);

   // Traversing u
   for (int i = 0; i < uDivisions; i++)
   {
      // Traversing v
      for (int j = 0; j < vDivisions; j++)
      {
         vec3 pos = vec3(r * sin(u) * cos(v),
            r * cos(u) * cos(v),
            r * sin(v));

         vec3 normal = normalize(pos - center);

         points.push_back(pos);
         normals.push_back(normal);
         v += radians(vStep);
      }

      u += radians(uStep);
   }
 
   for (int i = 0; i < uDivisions - 1; i++)
   {
      for (int j = 0; j < vDivisions - 1; j++)
      {
         unsigned int p00 = i*vDivisions + j;
         unsigned int p01 = i*vDivisions + j + 1;
         unsigned int p10 = (i + 1)* vDivisions + j;
         unsigned int p11 = (i + 1) * vDivisions + j + 1;

         indices.push_back(p00);
         indices.push_back(p10);
         indices.push_back(p01);

         indices.push_back(p01);
         indices.push_back(p10);
         indices.push_back(p11);
      }
   }
}

// create buffers and fill with geometry data, returning true if successful
bool InitializeGeometry(MyGeometry *geometry)
{
   vector<vec3> points;
   vector<vec3> normals;
   vector<unsigned int> indices;

   generateSphere(points, normals, indices, 1.0f, 48, 24);

   geometry->elementCount = indices.size();

   // these vertex attribute indices correspond to those specified for the
   // input variables in the vertex shader
   const GLuint VERTEX_INDEX = 0;
   const GLuint NORMAL_INDEX = 1;

   // create an array buffer object for storing our vertices
   glGenBuffers(1, &geometry->vertexBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*points.size(), points.data(), GL_STATIC_DRAW);

   //Buffer the vertex normals
   glGenBuffers(1, &geometry->normalBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, geometry->normalBuffer);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*normals.size(), normals.data(), GL_STATIC_DRAW);

   // create a vertex array object encapsulating all our vertex attributes
   glGenVertexArrays(1, &geometry->vertexArray);
   glBindVertexArray(geometry->vertexArray);

   // make element array buffer
   glGenBuffers(1, &geometry->elementBuffer);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->elementBuffer);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indices.size(), indices.data(), GL_STATIC_DRAW);

   // associate the position array with the vertex array object
   glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
   glVertexAttribPointer(VERTEX_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
   glEnableVertexAttribArray(VERTEX_INDEX);

   // Normal Buffer
   glBindBuffer(GL_ARRAY_BUFFER, geometry->normalBuffer);
   glVertexAttribPointer(NORMAL_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
   glEnableVertexAttribArray(NORMAL_INDEX);

   // unbind our buffers, resetting to default state
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindVertexArray(0);

   // check for OpenGL errors and return false if error occurred
   return !CheckGLErrors();
}

// deallocate geometry-related objects
void DestroyGeometry(MyGeometry *geometry)
{
   // unbind and destroy our vertex array object and associated buffers
   glBindVertexArray(0);
   glDeleteVertexArrays(1, &geometry->vertexArray);
   glDeleteBuffers(1, &geometry->vertexBuffer);
   glDeleteBuffers(1, &geometry->normalBuffer);
   glDeleteBuffers(1, &geometry->elementBuffer);
}

// --------------------------------------------------------------------------
// Rendering function that draws our scene to the frame buffer

void RenderScene(MyGeometry *geometry, MyShader *shader, mat4 proj, mat4 view, mat4 model)
{
   // bind our shader program and the vertex array object
   glUseProgram(shader->program);
   glBindVertexArray(geometry->vertexArray);

   // Get uniforms
   glUseProgram(shader->program);
   GLint modelUniform = glGetUniformLocation(shader->program, "model");
   GLint viewUniform = glGetUniformLocation(shader->program, "view");
   GLint projUniform = glGetUniformLocation(shader->program, "proj");

   glUniformMatrix4fv(modelUniform, 1, false, value_ptr(model));
   glUniformMatrix4fv(viewUniform, 1, false, value_ptr(view));
   glUniformMatrix4fv(projUniform, 1, false, value_ptr(proj));

   // tell OpenGL to draw our geometry
   glDrawElements(GL_TRIANGLES, geometry->elementCount, GL_UNSIGNED_INT, 0);

   // reset state to default (no shader or geometry bound)
   glBindVertexArray(0);
   glUseProgram(0);

   // check for an report any OpenGL errors
   CheckGLErrors();
}

// --------------------------------------------------------------------------
// GLFW callback functions

// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
   cout << "GLFW ERROR " << error << ":" << endl;
   cout << description << endl;
}

// handles keyboard input events
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   float move = 0.1f;
   float angle = 5.0f;

   if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
   {
      glfwSetWindowShouldClose(window, GL_TRUE);
   }
   else if (key == GLFW_KEY_E)
   {
      cam_.pos -= cam_.dir * move;
   }
   else if (key == GLFW_KEY_Q)
   {
      cam_.pos += cam_.dir * move;
   }
   else if (key == GLFW_KEY_D)
   {
      cam_.cameraRotation(radians(angle), 0.0f);
   }
   else if (key == GLFW_KEY_A)
   {
      cam_.cameraRotation(-radians(angle), 0.0f);
   }
   else if (key == GLFW_KEY_W)
   {
      cam_.cameraRotation(0.0f, radians(angle));
   }
   else if (key == GLFW_KEY_S)
   {
      cam_.cameraRotation(0.0f, -radians(angle));
   }
}

// ==========================================================================
// PROGRAM ENTRY POINT

int main(int argc, char *argv[])
{
   // initialize the GLFW windowing system
   if (!glfwInit()) {
      cout << "ERROR: GLFW failed to initialize, TERMINATING" << endl;
      return -1;
   }
   glfwSetErrorCallback(ErrorCallback);

   // attempt to create a window with an OpenGL 4.1 core profile context
   GLFWwindow *window = 0;
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   window = glfwCreateWindow(512, 512, "CPSC 453 OpenGL Assignment 5", 0, 0);
   if (!window) {
      cout << "Program failed to create GLFW window, TERMINATING" << endl;
      glfwTerminate();
      return -1;
   }

   // set keyboard callback function and make our context current (active)
   glfwSetKeyCallback(window, KeyCallback);
   glfwMakeContextCurrent(window);

   //Intialize GLAD
   if (!gladLoadGL())
   {
      cout << "GLAD init failed" << endl;
      return -1;
   }

   // query and print out information about our OpenGL environment
   QueryGLVersion();

   // call function to load and compile shader programs
   MyShader shader;
   if (!InitializeShaders(&shader)) {
      cout << "Program could not initialize shaders, TERMINATING" << endl;
      return -1;
   }

   // call function to create and fill buffers with geometry data
   MyGeometry geometry;
   if (!InitializeGeometry(&geometry))
      cout << "Program failed to intialize geometry!" << endl;   

   // Enable Depth Testing
   glEnable(GL_DEPTH_TEST);

   mat4  I(1);
   cam_ = Camera(vec3(0.f, 1.f, -1.f), vec3(0.f, 10.f, -10.f));

   // make a projection matrix   
   mat4 proj = perspective(radians(80.0f), 1.0f, 0.1f, 1000.0f);

   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   
   // Setup Models
   mat4 sunModel = translate(I, vec3(0.0f));
   mat4 earthModel = translate(sunModel, vec3(6.0f, 0.0f, 0.0f)) * scale(sunModel, vec3(0.65f, 0.65f, 0.65f));
   mat4 moonModel = translate(sunModel, vec3(2.0f, 0.0f, 0.0f)) * scale(earthModel, vec3(0.65f, 0.65f, 0.65f));

   // run an event-triggered main loop
   while (!glfwWindowShouldClose(window))
   {
      // clear screen to a dark grey colour
      glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
      glEnable(GL_DEPTH_TEST);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glUseProgram(shader.program);

      // make a view matrix
      mat4 view = cam_.getViewMatrix();

      // Sun 
      RenderScene(&geometry, &shader, proj, view, sunModel);

      // Earth
      RenderScene(&geometry, &shader, proj, view, earthModel);

      // Moon
      RenderScene(&geometry, &shader, proj, view, moonModel);

      glfwSwapBuffers(window);
      glfwPollEvents();
   }

   // clean up allocated resources before exit
   DestroyGeometry(&geometry);
   DestroyShaders(&shader);
   glfwDestroyWindow(window);
   glfwTerminate();

   cout << "Goodbye!" << endl;
   return 0;
}

// ==========================================================================
// SUPPORT FUNCTION DEFINITIONS

// --------------------------------------------------------------------------
// OpenGL utility functions

void QueryGLVersion()
{
   // query opengl version and renderer information
   string version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
   string glslver = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
   string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));

   cout << "OpenGL [ " << version << " ] "
      << "with GLSL [ " << glslver << " ] "
      << "on renderer [ " << renderer << " ]" << endl;
}

bool CheckGLErrors()
{
   bool error = false;
   for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
   {
      cout << "OpenGL ERROR:  ";
      switch (flag) {
      case GL_INVALID_ENUM:
         cout << "GL_INVALID_ENUM" << endl; break;
      case GL_INVALID_VALUE:
         cout << "GL_INVALID_VALUE" << endl; break;
      case GL_INVALID_OPERATION:
         cout << "GL_INVALID_OPERATION" << endl; break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
         cout << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
      case GL_OUT_OF_MEMORY:
         cout << "GL_OUT_OF_MEMORY" << endl; break;
      default:
         cout << "[unknown error code]" << endl;
      }
      error = true;
   }
   return error;
}

// --------------------------------------------------------------------------
// OpenGL shader support functions

// reads a text file with the given name into a string
string LoadSource(const string &filename)
{
   string source;

   ifstream input(filename.c_str());
   if (input) {
      copy(istreambuf_iterator<char>(input),
         istreambuf_iterator<char>(),
         back_inserter(source));
      input.close();
   }
   else {
      cout << "ERROR: Could not load shader source from file "
         << filename << endl;
   }

   return source;
}

// creates and returns a shader object compiled from the given source
GLuint CompileShader(GLenum shaderType, const string &source)
{
   // allocate shader object name
   GLuint shaderObject = glCreateShader(shaderType);

   // try compiling the source as a shader of the given type
   const GLchar *source_ptr = source.c_str();
   glShaderSource(shaderObject, 1, &source_ptr, 0);
   glCompileShader(shaderObject);

   // retrieve compile status
   GLint status;
   glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status);
   if (status == GL_FALSE)
   {
      GLint length;
      glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
      string info(length, ' ');
      glGetShaderInfoLog(shaderObject, info.length(), &length, &info[0]);
      cout << "ERROR compiling shader:" << endl << endl;
      cout << source << endl;
      cout << info << endl;
   }

   return shaderObject;
}

// creates and returns a program object linked from vertex and fragment shaders
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
   // allocate program object name
   GLuint programObject = glCreateProgram();

   // attach provided shader objects to this program
   if (vertexShader)   glAttachShader(programObject, vertexShader);
   if (fragmentShader) glAttachShader(programObject, fragmentShader);

   // try linking the program with given attachments
   glLinkProgram(programObject);

   // retrieve link status
   GLint status;
   glGetProgramiv(programObject, GL_LINK_STATUS, &status);
   if (status == GL_FALSE)
   {
      GLint length;
      glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &length);
      string info(length, ' ');
      glGetProgramInfoLog(programObject, info.length(), &length, &info[0]);
      cout << "ERROR linking shader program:" << endl;
      cout << info << endl;
   }

   return programObject;
}