
#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
  
#include <GL/glew.h> // Include glew to get all the required OpenGL headers

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
