#pragma once
#include <string>

//Shader utility functions for compilation
GLuint buildShader(int type, char* filename);
GLuint buildProgram(std::string vs, std::string fs, int first, ...);
void dumpProgram(int program, char* desc);

GLuint buildTexture(std::string filename);