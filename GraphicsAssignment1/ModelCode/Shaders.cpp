#ifdef WIN32
#include <Windows.h>
#endif
#include <GL/glew.h>
#include <stdio.h>
#include <stdarg.h>

#include "Shaders.h"

static char* readShaderFile(char* filename) {
	FILE* fid;
	char* buffer;
	int len, n;

	fopen_s(&fid, filename, "r");
	if (!fid) {
		printf("Can't open shader file: %s\n", filename);
		return 0;
	}

	fseek(fid, 0, SEEK_END);
	len = ftell(fid);
	rewind(fid);

	buffer = new char[len + 1];
	n = fread(buffer, sizeof(char), len, fid);
	buffer[n] = 0;

	fclose(fid);

	return buffer;
}

static GLuint buildShader(int type, char* filename) {
	int result, shader = glCreateShader(type);
	char* source = readShaderFile(filename);
	if (source == 0) return 0; //couldn't open file

	glShaderSource(shader, 1, (const GLchar**)&source, 0);
	glCompileShader(shader);
	
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result); //error handling
	if (!result) {
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		if (type == GL_VERTEX_SHADER) printf("Error: vertex shader failed compilation:\n%s\n", infoLog);
		else if (type == GL_FRAGMENT_SHADER) printf("Error: fragment shader failed compilation:\n%s\n", infoLog);
		else printf("Error: unknown shader failed compilation:\n%s\n", infoLog);
	}

	return shader;
}

GLuint buildProgram(std::string vs, std::string fs, int first, ...) {
	int result, shader;
	unsigned int program = glCreateProgram();
	va_list argptr;

	glAttachShader(program, buildShader(GL_VERTEX_SHADER, (char*)vs.data()));
	glAttachShader(program, buildShader(GL_FRAGMENT_SHADER, (char*)fs.data()));

	glAttachShader(program, first);
	va_start(argptr, first);
	while ((shader = va_arg(argptr, int)) != 0) glAttachShader(program, shader);

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &result);
	if (!result) {
		char infoLog[512];
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		printf("Error: Program link error:\n%s\n", infoLog);
	}

	return program;
}

static void dumpProgram(int program, char* desc) {
	char name[256];
	GLsizei length;
	GLint size;
	GLenum type;
	int uniforms, attributes, shaders;

	printf("Information for shaders: %s\n", desc);

	if (!glIsProgram(program)) {
		printf("Error: Not a valid shader program.\n");
		return;
	}

	glGetProgramiv(program, GL_ATTACHED_SHADERS, &shaders);
	printf("Number of shaders: %d\n", shaders);

	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniforms);
	printf("Uniforms: %d\n", uniforms);
	for (int i = 0; i < uniforms; i++) {
		glGetActiveUniform(program, i, 256, &length, &size, &type, name);
		printf("\tName: %s\n", name);
	}
	glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &attributes);
	printf("Attributes: %d\n", attributes);
	for (int i = 0; i < attributes; i++) {
		glGetActiveAttrib(program, i, 256, &length, &size, &type, name);
		printf("\tName: %s\n", name);
	}
}