#pragma once
#include <string>
//Shader utility functions for compilation
static GLuint buildShader(int type, char* filename);
GLuint buildProgram(std::string vs, std::string fs, int first, ...); //why does making this static break it?
static void dumpProgram(int program, char* desc);