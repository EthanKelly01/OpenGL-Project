#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <vector>

#include "tiny_obj_loader.h"

//performance stuff
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;

	Vertex() : position(0.0f), normal(0.0f), texCoords(0.0f) {};
	Vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 texCoords) : position(position), normal(normal), texCoords(texCoords) {};
};

struct Texture {
	GLuint id;
	std::string type;
};

struct Mesh {
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;
};

class Model {
public:
	Model(const std::string& filename, int shader = -1);

	void addShader(GLuint shader) { this->shader = shader; }
	void addTexture(Texture tex) { this->mesh.textures.push_back(tex); }
	void addTexture(std::vector<Texture> tex) { mesh.textures.insert(mesh.textures.end(), tex.begin(), tex.end()); }

	const glm::vec3 scaleModel(glm::vec3 size);
	const glm::vec4 rotateModel(glm::vec3 dir);

	void createInstance(glm::vec3 position = {0, 0, 0}, glm::vec3 direction = glm::vec3{0, 0, 0}, glm::vec3 scale = {0, 0, 0});
	void updateInstance(int index, glm::vec3 position, glm::vec3 direction, glm::vec3 scale = {0, 0, 0});

	void draw();

	Mesh* getMesh() { return &mesh; }
	GLuint getShader() { return shader; }
	GLuint getVAO() { return VAO; }

	std::string getName() { return filename; }
	std::string getPath() { return filepath; }

	void debug() {
		printf("Model file: %s\n", filename.c_str());
		printf("Model raw scale: %f, %f, %f\n", size.x, size.y, size.z);
		printf("Number of instances: %I64d\n********************************************\n", instances.size());
		for (int i = 0; i < instances.size(); i++) {
			glm::vec3 newScale = scalars[i] * size;
			printf("\nInstance %d\n", i);
			printf("Instance position: %f, %f, %f\n", positions[i].x, positions[i].y, positions[i].z);
			printf("Instance direction: %f, %f, %f\n", directions[i].x, directions[i].y, directions[i].z);
			printf("Instance scale: %f, %f, %f\n", newScale.x, newScale.y, newScale.z);
			printf("********************************************\n");
		}
	}
private:
	Mesh mesh;
	GLuint VAO, VBO, EBO, shader = -1;

	// v debug info v
	std::string filepath, filename, bin;
	glm::vec3 size;
	std::vector<glm::mat4> instances;
	std::vector<glm::vec3> scalars;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> directions;
	
	void setupMesh();
};