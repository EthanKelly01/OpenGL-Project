#define _CRT_SECURE_NO_WARNINGS
#include "Model.h"

Model::Model(const std::string& filepath, int shader) {
	this->filepath = filepath;
	this->bin = filepath.substr(0, filepath.find_last_of(".") + 1).append("bin");
	this->filename = filepath.substr(filepath.find_last_of("\\") + 1, filepath.find_last_of(".") - filepath.find_last_of("\\") - 1);
	std::string extension = filepath.substr(filepath.find_last_of(".") + 1);

	if (shader >= 0) this->shader = shader;

	int nv, ni, result;
	struct _stat buf;
	if (extension == "bin" || _stat(bin.c_str(), &buf) == 0) {
		//bin found
		int file = _open(bin.c_str(), _O_RDONLY | _O_BINARY);
		result = _read(file, &nv, (sizeof nv));
		result = _read(file, &ni, (sizeof ni));
		result = _read(file, &size, (sizeof size));

		GLfloat* vertices = new GLfloat[nv * 8];
		mesh.vertices.reserve(nv);
		result = _read(file, vertices, (nv * 8) * (sizeof GLfloat));
		
		int j = 0;
		for (int i = 0; i < nv; i++) {
			mesh.vertices.emplace_back();
			mesh.vertices[i].position = { vertices[j++], vertices[j++], vertices[j++] };
			mesh.vertices[i].normal = { vertices[j++], vertices[j++], vertices[j++] };
			mesh.vertices[i].texCoords = { vertices[j++], vertices[j++] };
		}

		GLuint* indices = new GLuint[ni];
		result = _read(file, indices, ni * (sizeof GLuint));
		for (int i = 0; i < ni; i++) mesh.indices.emplace_back(indices[i]);
		_close(file);
	} else if (extension == "obj") {
		//load obj and create bin
		tinyobj::ObjReaderConfig reader_config;
		reader_config.mtl_search_path = ""; // Path to material files
		tinyobj::ObjReader reader;

		if (!reader.ParseFromFile(filepath, reader_config)) {
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

			if (shapes[0].mesh.indices[i].normal_index >= 0) mesh.vertices.back().normal = glm::vec3(attrib.normals[size_t(i)], attrib.normals[size_t(i) + 1], attrib.normals[size_t(i) + 2]);
			if (shapes[0].mesh.indices[i].texcoord_index >= 0) mesh.vertices.back().texCoords = glm::vec2(attrib.texcoords[size_t(i / 3 * 2)], attrib.texcoords[size_t(i / 3 * 2) + 1]);
		}

		size = glm::vec3(xmax - xmin, ymax - ymin, zmax - zmin);

		//normalize min x/y/z to 0/0/0
		for (Vertex vert : mesh.vertices) {
			vert.position.x -= xmin;
			vert.position.y -= ymin;
			vert.position.z -= zmin;
		}

		mesh.indices.clear();
		mesh.indices.reserve(shapes[0].mesh.indices.size());
		for (int i = 0; i < shapes[0].mesh.indices.size(); i++) mesh.indices.emplace_back(shapes[0].mesh.indices[i].vertex_index);

		//save bin
		nv = mesh.vertices.size();
		ni = mesh.indices.size();
		GLfloat* vertices = new GLfloat[nv * 8];
		int j = 0;
		for (int i = 0; i < nv; i++) {
			vertices[j++] = mesh.vertices[i].position.x;
			vertices[j++] = mesh.vertices[i].position.y;
			vertices[j++] = mesh.vertices[i].position.z;
			vertices[j++] = mesh.vertices[i].normal.x;
			vertices[j++] = mesh.vertices[i].normal.y;
			vertices[j++] = mesh.vertices[i].normal.z;
			vertices[j++] = mesh.vertices[i].texCoords.x;
			vertices[j++] = mesh.vertices[i].texCoords.y;
		}

		GLuint* indices = new GLuint[ni];
		for (int i = 0; i < ni; i++) indices[i] = mesh.indices[i];

		int file = _open(bin.c_str(), _O_WRONLY | _O_BINARY | _O_CREAT, _S_IREAD | _S_IWRITE);
		_write(file, &nv, (sizeof nv));
		_write(file, &ni, (sizeof ni));
		_write(file, &size, (sizeof size));
		_write(file, vertices, (unsigned long long)nv * 8 * (sizeof GLfloat));
		_write(file, indices, ni * (sizeof GLuint));
		_close(file);
	} else printf("Error: Can't load that filetype as a model.\n");

	//setup mesh
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

const glm::vec3 Model::scaleModel(glm::vec3 newSize) {
	if (newSize == glm::vec3{0, 0, 0}) return size;
	glm::vec3 result = glm::vec3(0.0f, 0.0f, 0.0f);

	if (newSize.x != 0) { //find first non zero value in newSize
		result.x = newSize.x / size.x;
		(newSize.y == 0) ? result.y = result.x : result.y = newSize.y / size.y;
		(newSize.z == 0) ? result.z = result.x : result.z = newSize.z / size.z;
	} else if (newSize.y != 0) {
		result.x = result.y = newSize.y / size.y;
		(newSize.z == 0) ? result.z = result.x : result.z = newSize.z / size.z;
	} else result.x = result.y = result.z = newSize.z / size.z;

	return result;
}

const glm::vec4 Model::rotateModel(glm::vec3 dir) {
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

void Model::instance(glm::vec3 position, glm::vec3 direction, glm::vec3 scale, int index) {
	if (index == -1) {
		if (shader == -1) {
			printf("No applied shader.\n");
			return;
		}
		instances.push_back({ position, direction, scale });
		glUseProgram(shader);

		GLuint vbuffer, ibuffer;
		GLfloat colours[] = { 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0 };

		int nv = mesh.vertices.size();
		int ni = mesh.indices.size();
		GLfloat* vertices = new GLfloat[nv * 8];
		int j = 0;
		for (int i = 0; i < nv; i++) {
			vertices[j++] = mesh.vertices[i].position.x;
			vertices[j++] = mesh.vertices[i].position.y;
			vertices[j++] = mesh.vertices[i].position.z;
			vertices[j++] = mesh.vertices[i].normal.x;
			vertices[j++] = mesh.vertices[i].normal.y;
			vertices[j++] = mesh.vertices[i].normal.z;
			vertices[j++] = mesh.vertices[i].texCoords.x;
			vertices[j++] = mesh.vertices[i].texCoords.y;
		}

		glGenBuffers(1, &vbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vbuffer);

		glBufferData(GL_ARRAY_BUFFER, j * (sizeof GLfloat) + (sizeof colours), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, j * (sizeof GLfloat), vertices);
		glBufferSubData(GL_ARRAY_BUFFER, j * (sizeof GLfloat), sizeof colours, colours);

		GLuint location = glGetAttribLocation(shader, "vPosition");
		glVertexAttribPointer(location, 3, GL_FLOAT, 0, 8 * (sizeof GLfloat), 0);
		glEnableVertexAttribArray(location);

		location = glGetAttribLocation(shader, "vNormal");
		glVertexAttribPointer(location, 3, GL_FLOAT, 0, 8 * (sizeof GLfloat), (void*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(location);

		location = glGetAttribLocation(shader, "vTexture");
		glVertexAttribPointer(location, 2, GL_FLOAT, 0, 8 * (sizeof GLfloat), (void*)(6 * sizeof(GLfloat)));
		glEnableVertexAttribArray(location);

		location = glGetAttribLocation(shader, "vColour");
		glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, 0, (void*)(j * sizeof(GLfloat)));
		glEnableVertexAttribArray(location);
		glVertexAttribDivisor(location, 1);

		glGenBuffers(1, &ibuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer);

		GLuint* indices = new GLuint[mesh.indices.size()];
		for (int i = 0; i < mesh.indices.size(); i++) indices[i] = mesh.indices[i];
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(GLuint), indices, GL_STATIC_DRAW);
	} else if (index < instances.size()) instances[index] = { position, direction, scale };
	else printf("Error: no object with that instance index.\n");

	glm::vec4 rotation = rotateModel(instances[0][1]);
	transform = glm::translate(transform, instances[0][0]);
	transform = glm::rotate(transform, rotation.w, glm::vec3(rotation.x, rotation.y, rotation.z));
	transform = glm::scale(transform, scaleModel(instances[0][2]));
}

void Model::draw() {
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

	glUniformMatrix4fv(glGetUniformLocation(shader, "transform"), 1, 0, glm::value_ptr(transform));

	// draw mesh
	glBindVertexArray(VAO);
	glDrawElementsInstanced(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, NULL, instances.size());
	glActiveTexture(GL_TEXTURE0); //sets current texture back to 0
}