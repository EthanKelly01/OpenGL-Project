#include "Model.h"

Model::Model(const std::string& filename, int shader) {
	this->filename = filename;

	if (shader >= 0) this->shader = shader;

	tinyobj::ObjReaderConfig reader_config;
	reader_config.mtl_search_path = ""; // Path to material files
	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(filename, reader_config)) {
		if (!reader.Error().empty()) printf("TinyObjReader: %s\n", reader.Error().c_str());
		exit(1);
	}
	if (!reader.Warning().empty()) printf("TinyObjReader: %s\n", reader.Warning().c_str());

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	auto& materials = reader.GetMaterials();

	mesh.vertices.clear();
	mesh.vertices.reserve(attrib.vertices.size() / 3);

	float xmin = attrib.vertices[0], xmax = xmin, ymin = attrib.vertices[1], ymax = ymin, zmin = attrib.vertices[2], zmax = zmin;

	for (int i = 0; i < attrib.vertices.size(); i += 3) {
		mesh.vertices.emplace_back();
		mesh.vertices.back().position = glm::vec3(attrib.vertices[size_t(i)], attrib.vertices[size_t(i) + 1], attrib.vertices[size_t(i) + 2]);

		//finding object scale
		if (attrib.vertices[size_t(i)] < xmin) xmin = attrib.vertices[size_t(i)];
		else if (attrib.vertices[size_t(i)] > xmax) xmax = attrib.vertices[size_t(i)];
		if (attrib.vertices[size_t(i) + 1] < ymin) ymin = attrib.vertices[size_t(i) + 1];
		else if (attrib.vertices[size_t(i) + 1] > ymax) ymax = attrib.vertices[size_t(i) + 1];
		if (attrib.vertices[size_t(i) + 2] < zmin) zmin = attrib.vertices[size_t(i) + 2];
		else if (attrib.vertices[size_t(i) + 2] > zmax) zmax = attrib.vertices[size_t(i) + 2];

		if (shapes[0].mesh.indices[i].normal_index >= 0)
			mesh.vertices.back().normal = glm::vec3(attrib.normals[size_t(i)], attrib.normals[size_t(i) + 1], attrib.normals[size_t(i) + 2]);

		if (shapes[0].mesh.indices[i].texcoord_index >= 0)
			mesh.vertices.back().texCoords = glm::vec2(attrib.texcoords[size_t(i / 3 * 2)], attrib.texcoords[size_t(i / 3 * 2) + 1]);

		// Optional: vertex colors
		// tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
		// tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
		// tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];

		//shapes[0].mesh.material_ids[i / 3]; //material for each face
	}

	size = glm::vec3(xmax - xmin, ymax - ymin, zmax - zmin);

	//normalize min x/y/z to 0/0/0
	for (Vertex vert : mesh.vertices) {
		vert.position.x -= xmin;
		vert.position.y -= ymin;
		vert.position.z -= zmin;
	}

	/*for (int i = 0; i < shapes[0].mesh.num_face_vertices.size(); i++) { //foreach face
		for (int j = 0; j < shapes[0].mesh.num_face_vertices[i]; j++) { //foreach vertex in face

		}
	}*/

	mesh.indices.clear();
	mesh.indices.reserve(shapes[0].mesh.indices.size());
	for (int i = 0; i < shapes[0].mesh.indices.size(); i++) mesh.indices.emplace_back(shapes[0].mesh.indices[i].vertex_index);

	setupMesh();
}

const glm::vec3 Model::scaleModel(glm::vec3 newSize) {
	if (newSize.x == 0 && newSize.y == 0 && newSize.z == 0) return newSize;
	glm::vec3 result = glm::vec3(0.0f, 0.0f, 0.0f);

	if (newSize.x != 0) { //find first non zero value in newSize
		result.x = newSize.x / size.x;
		if (newSize.y == 0) result.y = result.x;
		else result.y = newSize.y / size.y;
		if (newSize.z == 0) result.z = result.x;
		else result.z = newSize.z / size.z;
	} else if (newSize.y != 0) {
		result.x = result.y = newSize.y / size.y;
		if (newSize.z == 0) result.z = result.x;
		else result.z = newSize.z / size.z;
	} else result.x = result.y = result.z = newSize.z / size.z;

	return result;
}

const glm::vec4 Model::rotateModel(glm::vec3 dir) {
	//direction is yaw, pitch, and roll
	/*float yaw = dir.x;
	float pitch = dir.y;
	float roll = dir.z;

	glm::vec3 front = glm::normalize(glm::vec3(cos(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(pitch)), sin(glm::radians(yaw)) * cos(glm::radians(pitch))));
	glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f))); //reculculate camera right and up
	glm::vec3 up = glm::normalize(glm::cross(right, front));*/

	glm::vec3 from_vector = glm::vec3(1.0f);
	glm::normalize(from_vector);
	glm::normalize(dir);
	float cosa = glm::dot(from_vector, dir);
	glm::clamp(cosa, -1.0f, 1.0f);
	glm::vec3 axis = glm::cross(from_vector, dir);
	float angle = glm::degrees(glm::acos(cosa));
	if ((axis.x != 0 || axis.y != 0 || axis.z != 0) && (angle >= 0 || angle < 0)) return glm::vec4(axis, angle);
	return glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
}

void Model::createInstance(glm::vec3 position, glm::vec3 direction, glm::vec3 scale) {
	instances.push_back(glm::mat4(1.0f));
	scalars.push_back(glm::vec3(0.0f));
	positions.push_back(glm::vec3(0.0f));
	directions.push_back(glm::vec3(0.0f));
	updateInstance(instances.size() - 1, position, direction, scale);
}

void Model::updateInstance(int index, glm::vec3 position, glm::vec3 direction, glm::vec3 scale) {
	glm::vec4 rotation = rotateModel(direction);
	instances[index] = glm::translate(instances[index], position);
	instances[index] = glm::rotate(instances[index], rotation.w, glm::vec3(rotation.x, rotation.y, rotation.z));
	instances[index] = glm::scale(instances[index], scaleModel(scale));

	scalars[index] = scaleModel(scale);
	positions[index] = position;
	directions[index] = direction;
}

void Model::updateInstance(int index, glm::vec3 position, glm::vec3 direction) {
	glm::vec4 rotation = rotateModel(direction);
	instances[index] = glm::translate(instances[index], position);
	instances[index] = glm::rotate(instances[index], rotation.w, glm::vec3(rotation.x, rotation.y, rotation.z));

	positions[index] = position;
	directions[index] = direction;
}

void Model::draw() {
	for (glm::mat4 instance : instances) {
		if (!shader) {
			printf("Can't draw without a valid shader.\n");
			return;
		}

		unsigned int diffuseNr = 0, specularNr = 0, general = 0;
		for (unsigned int i = 0; i < mesh.textures.size(); i++) {
			glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
			// retrieve texture number (the N in diffuse_textureN)
			std::string number, name = mesh.textures[i].type;
			if (name == "texture_diffuse") number = std::to_string(diffuseNr++);
			else if (name == "texture_specular") number = std::to_string(specularNr++);
			else number = std::to_string(general++);

			glUniform1i(glGetUniformLocation(shader, ("material" + name + number).c_str()), i);
			glBindTexture(GL_TEXTURE_2D, mesh.textures[i].id);
		}

		glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(instance));

		// draw mesh
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glActiveTexture(GL_TEXTURE0); //sets current texture back to 0
	}
}

void Model::setupMesh() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), &mesh.vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), &mesh.indices[0], GL_STATIC_DRAW);

	// vertex positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);
	// vertex normals
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(1);
	// vertex texture coords
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}