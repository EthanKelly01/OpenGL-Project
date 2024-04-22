#pragma once

#include "ModelCode/stb_image.h"

static GLuint buildTexture(std::string filename) {
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