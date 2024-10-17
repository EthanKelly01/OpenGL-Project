#ifdef WIN32
#include <Windows.h>
#endif
#include <GL/glew.h>
#include <stdio.h>
#include <stdarg.h>

#include "Shaders.h"
#include "stb_image.h"

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

GLuint buildShader(int type, char* filename) {
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

void dumpProgram(int program, char* desc) {
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

GLuint buildTexture(std::string filename) {
	GLuint tex;

	stbi_set_flip_vertically_on_load(true);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load and generate the texture
	int width, height, nrChannels;
	unsigned char* data = stbi_load((char*)filename.data(), &width, &height, &nrChannels, 0);
	if (data) {
		size_t pos = filename.rfind('.');
		if (pos == std::string::npos) return -1; //no extension detected

		std::string ext = filename.substr(pos + 1); //check if file has alpha colour channel
		if (ext == "jpg") glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		else if (ext == "png") glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else printf("Error: Failed to load texture.\n");

	stbi_image_free(data);

	return tex;
}