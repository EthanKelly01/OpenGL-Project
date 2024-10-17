#pragma once

#include "imgui.h"

bool assetManager = 0, modelWindow = 0;
extern GLFWwindow* window;
extern Scene scene;

glm::vec3 position = { 0, 0, 0 };
std::string vs, fs;

std::string projectDialog() {
	char const* lFilterPatterns[1] = { "*.scn" };
	char const* selection = tinyfd_openFileDialog("Select file", "C:\\", 1, lFilterPatterns, NULL, 0);
	return selection ? selection : "";
}

void renderUI() {
	//ImGui::ColorEdit4("Color", my_color); // Edit a color stored as 4 floats

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New", "Ctrl+N"));// project = *new Project(projectDialog());
			if (ImGui::MenuItem("Open", "Ctrl+O")) scene = Scene(projectDialog());
			if (ImGui::MenuItem("Save", "Ctrl+S")) scene.save();
			if (ImGui::MenuItem("Exit", "Alt+f4")) glfwSetWindowShouldClose(window, 1);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Scene")) {
			if (ImGui::MenuItem("New"));// project.scenes.push_back(Scene("Test"));
			//for (Scene scene : project.scenes) {
				//if (ImGui::MenuItem(scene.name.c_str()));
			//}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Window")) {
			if (ImGui::MenuItem("Asset Manager")) assetManager = !assetManager;
			if (ImGui::MenuItem("Model Manager")) modelWindow = !modelWindow;
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	//asset manager
	if (assetManager) {
		ImGui::Begin("Assets", &assetManager, ImGuiWindowFlags_MenuBar);
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Open..", "Ctrl+O")) {
					const char* lFilterPatterns[7] = { "*.txt", "*.png", "*.jpg", "*.obj", "*.bin", "*.vs", "*.fs"};
					const char* ret = tinyfd_openFileDialog("Select file", "C:\\", 7, lFilterPatterns, NULL, 1);
					std::string files(ret == nullptr ? std::string() : ret);

					size_t pos = 0;
					while ((pos = files.find("|")) != std::string::npos) {
						scene.add(files.substr(0, pos));
						files.erase(0, pos + 1);
					}
					scene.add(files);
				}
				if (ImGui::MenuItem("Save", "Ctrl+S")) {}
				if (ImGui::MenuItem("Close", "Ctrl+W")) { assetManager = 0; }
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
		ImGui::BeginChild("Scrolling");
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Models");
		if (scene.models.empty()) ImGui::Text("No models loaded.");
		else for (int i = 0; i < scene.models.size(); i++) {
			ImGui::Text(scene.models[i].getName().c_str());
			ImGui::PushID(i);
			ImGui::SameLine(ImGui::GetWindowWidth() - 50);
			if (ImGui::Button("Remove##0")) scene.models.erase(scene.models.begin() + i); //remove file
			ImGui::PopID();
		}
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Shaders");
		if (scene.shaderSources.empty()) ImGui::Text("No shaders loaded.");
		else for (int i = 0; i < scene.shaderSources.size(); i++) {
			ImGui::Text(scene.shaderSources[i].first.c_str());
			ImGui::PushID(i);
			ImGui::SameLine(ImGui::GetWindowWidth() - 50);
			if (ImGui::Button("Remove##1")) scene.shaderSources.erase(scene.shaderSources.begin() + i); //remove file
			ImGui::PopID();
		}
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Textures");
		if (scene.textures.empty()) ImGui::Text("No textures loaded.");
		else for (int i = 0; i < scene.textures.size(); i++) {
			ImGui::Text(scene.textures[i].first.c_str());
			ImGui::PushID(i);
			ImGui::SameLine(ImGui::GetWindowWidth() - 50);
			if (ImGui::Button("Remove##2")) scene.textures.erase(scene.textures.begin() + i); //remove file
			ImGui::PopID();
		}
		ImGui::EndChild();
		
		ImGui::End();
	}

	if (modelWindow && !scene.models.empty()) {
		ImGui::Begin(scene.models[0].getName().c_str(), &modelWindow, ImGuiWindowFlags_MenuBar);
		if (ImGui::BeginMenuBar()) {
			ImGui::EndMenuBar();
		}
		ImGui::Text("Filepath: %s", scene.models[0].getPath().c_str());
		ImGui::Text("Shaders:");
		for (int i = 0; i < scene.shaders.size(); i++) {
			ImGui::Text(scene.shaders[i].first.c_str());
		}
		ImGui::Text("Add Shaders:");
		for (int i = 0; i < scene.shaderSources.size(); i++) {
			ImGui::Text(scene.shaderSources[i].first.c_str());
			ImGui::PushID(i);
			ImGui::SameLine(ImGui::GetWindowWidth() - 50);
			if (ImGui::Button("Add")) {
				if (scene.shaderSources[i].second.substr(scene.shaderSources[i].second.find_last_of(".") + 1) == "vs") vs = scene.shaderSources[i].second;
				if (scene.shaderSources[i].second.substr(scene.shaderSources[i].second.find_last_of(".") + 1) == "fs") fs = scene.shaderSources[i].second;
			}
			ImGui::PopID();
		}
		if (ImGui::Button("Compile") && !vs.empty() && !fs.empty()) {
			scene.shaders.emplace_back("test", buildProgram(vs, fs, 0));
			scene.models[0].addShader(scene.shaders.back().second);
		}
		ImGui::SameLine();
		if (ImGui::Button("Create")) scene.models[0].createInstance({0, 0, 0}, {0, 0, 0}, {0, 5, 0});
		ImGui::End();
	}
}