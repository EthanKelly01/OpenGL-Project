#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <GL/glew.h>
#include "Model.h"
#include "Shaders.h"
#include "tfd/tinyfiledialogs.h"

class Scene {
public:
	std::vector<Model> models;
	std::vector<std::pair<std::string, std::string>> shaderSources;
	std::vector<std::pair<std::string, GLuint>> shaders;
	std::vector<std::pair<std::string, GLuint>> textures;

	Scene() {};
	Scene(std::string filepath);

	void add(std::string filepath);
	void save();
private:
	std::string filepath;
};