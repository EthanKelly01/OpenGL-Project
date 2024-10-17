#define _CRT_SECURE_NO_WARNINGS
#include "FileSystem.h"

std::string saveDialog() {
	char const* lFilterPatterns[1] = { "*.scn" };
	char const* selection = tinyfd_saveFileDialog("Select file", "C:\\", 1, lFilterPatterns, NULL);
	return selection ? selection : "";
}

Scene::Scene(std::string filepath) {
	this->filepath = filepath;
	struct _stat buf;
	if (_stat(filepath.c_str(), &buf) == 0) {
		int file = _open(filepath.c_str(), _O_RDONLY | _O_BINARY);

		int result, nm, nss, ns, nt;
		result = _read(file, &nm, (sizeof nm));
		result = _read(file, &nss, (sizeof nss));
		result = _read(file, &ns, (sizeof ns));
		result = _read(file, &nt, (sizeof nt));

		models.reserve(nm);
		shaderSources.reserve(nss);
		shaders.reserve(ns);
		textures.reserve(nt);

		//result = _read(file, &models, nm * (sizeof Model));
		//result = _read(file, &shaderSources, nss * (sizeof std::pair<std::string, std::string>));
		//result = _read(file, &shaders, ns * (sizeof std::pair<std::string, GLuint>));
		//result = _read(file, &textures, nt * (sizeof std::pair<std::string, GLuint>));

		_close(file);
	} else printf("Error, couldn't open save file.\n");
}

void Scene::add(std::string filepath) {
	std::string filename = filepath.substr(filepath.find_last_of("\\") + 1, filepath.find_last_of(".") - filepath.find_last_of("\\") - 1);
	std::string extension = filepath.substr(filepath.find_last_of(".") + 1);

	if (extension == "obj" || extension == "bin") models.emplace_back(filepath);
	else if (extension == "vs" || extension == "fs") shaderSources.emplace_back(filename.append("." + extension), filepath);
	else if (extension == "png" || extension == "jpg") textures.emplace_back(filename, buildTexture(filepath));
	else printf("Sorry, don't know what that filetype is.\n");
}

void Scene::save() {
	while (filepath.empty()) filepath = saveDialog();
	
	int file = _open(filepath.c_str(), _O_WRONLY | _O_BINARY | _O_CREAT, _S_IREAD | _S_IWRITE);
	int nm = models.size();
	int nss = shaderSources.size();
	int ns = shaders.size();
	int nt = textures.size();

	_write(file, &nm, (sizeof nm));
	_write(file, &nss, (sizeof nss));
	_write(file, &ns, (sizeof ns));
	_write(file, &nt, (sizeof nt));

	//_write(file, &models, nm * (sizeof Model));
	//_write(file, &shaderSources, nss * (sizeof std::pair<std::string, std::string>));
	//_write(file, &shaders, ns * (sizeof std::pair<std::string, GLuint>));
	//_write(file, &textures, nt * (sizeof std::pair<std::string, GLuint>));

	_close(file);
}

//tinyfiledialog example
/*char const* lFilterPatterns[2] = { "*.txt", "*.jpg" };
	// there is also a wchar_t version
	char const* selection = tinyfd_openFileDialog(
		"Select file", // title
		"C:\\", // optional initial directory
		2, // number of filter patterns
		lFilterPatterns, // char const * lFilterPatterns[2] = { "*.txt", "*.jpg" };
		NULL, // optional filter description
		0 // forbids multiple selections
	);*/