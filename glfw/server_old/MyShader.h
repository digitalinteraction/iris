
#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
  
#include <GL/glew.h> // Include glew to get all the required OpenGL headers

#define CHECK_GL_ERRORS
#if defined(CHECK_GL_ERRORS)
#define GLCHK(X) \
do { \
    GLenum err = GL_NO_ERROR; \
    X; \
   while ((err = glGetError())) \
   { \
      printf("GL error 0x%x in " #X " file %s line %d\n", err, __FILE__,__LINE__); \
      exit(err); \
   } \
} \
while(0)
#else
#define GLCHK(X) X
#endif /* CHECK_GL_ERRORS */


class MyShader
{
public:
  	// The program ID
	GLuint Program;
	// Constructor reads and builds the shader
	MyShader(const GLchar* vertexPath, const GLchar* fragmentPath);
  	// Use the program
  	void Use();
private:
};
  
#endif
