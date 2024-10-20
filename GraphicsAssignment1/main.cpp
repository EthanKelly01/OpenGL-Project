#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "tfd/tinyfiledialogs.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Shaders.h"
#include "Model.h"

#include "Camera.h"
#include "FileSystem.h"
#include "GUI.h"

float windowWidth = 1600.0f, windowHeight = 1200.0f; //startup size

GLFWwindow* window;
Scene scene;
FlyCam camera(windowWidth, windowHeight);

float deltaTime = 0.0f, lastFrame = 0.0f; //timing

bool menu = 0, prevEsc = 0, firstMouse = 1, lastDebug = 0; //input stuff
float lastX = windowWidth / 2, lastY = windowHeight / 2;

static void resizeWindow(GLFWwindow* window, int width, int height) { 
	glViewport(0, 0, width, height);
	camera.resizeWindow(windowWidth = width, windowHeight = height);
}

static void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
		if (!lastDebug) {
			camera.debug();
			for (Model model : scene.models) model.debug();
			lastDebug = 1;
		}
	} else lastDebug = 0;

	if (menu) {
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			menu = false;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			firstMouse = 1;
		} else if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			if (!prevEsc) glfwSetWindowShouldClose(window, 1);
		} else prevEsc = 0;
	}
	if (menu) return;
	
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		menu = prevEsc = 1;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		return;
	}

	bool first; //saves a function call checking xor for opposing movements
	std::vector<Camera_Movement> mov;
	if ((first = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) != (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)) {
		if (first) mov.push_back(FORWARD);
		else mov.push_back(BACKWARD);
	}
	if ((first = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) != (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)) {
		if (first) mov.push_back(LEFT);
		else mov.push_back(RIGHT);
	}
	if ((first = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) != (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)) {
		if (first) mov.push_back(UP);
		else mov.push_back(DOWN);
	}
	if (mov.size() > 0) {
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camera.setSprint();
		camera.setTime(deltaTime);
		camera.setMovement(mov);
	}

	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) camera.setOrtho(1);
}

static void mouseInput(GLFWwindow* window, double xpos, double ypos) {
	if (menu) return;

	if (firstMouse) firstMouse = false;
	else camera.processMouse(xpos - lastX, lastY - ypos);

	lastX = xpos;
	lastY = ypos;
}

static void scroll(GLFWwindow* window, double xoffset, double yoffset) { if (!menu) camera.processScroll(static_cast<float>(yoffset)); }

int main() {
	glfwInit(); //init and configure glfw (version 3.3 core)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //create and configure window
	window = glfwCreateWindow(windowWidth, windowHeight, "MyProject", NULL, NULL);
	if (!window) {
		printf("Error: Failed to create GLFW window.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetFramebufferSizeCallback(window, resizeWindow);
	glfwSetCursorPosCallback(window, mouseInput);
	glfwSetScrollCallback(window, scroll);
		
	if (glewInit() != GLEW_OK || !GLEW_VERSION_2_1) exit(1); //init glew
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.5f, 1.0f, 1.0f); //rgba (alpha is opacity)

	//imgui setup
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	//main loop
	while (!glfwWindowShouldClose(window)) {
		deltaTime = glfwGetTime() - lastFrame;
		lastFrame = glfwGetTime();

		processInput(window);
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //just clears to some colour

		std::pair<glm::mat4*, glm::mat4*> matrices = camera.getMatrices(); //updating shaders with current camera matrices
		for (std::pair<std::string, GLuint> shader : scene.shaders) {
			glUseProgram(shader.second);
			glUniformMatrix4fv(glGetUniformLocation(shader.second, "view"), 1, 0, glm::value_ptr(*matrices.second));
			glUniformMatrix4fv(glGetUniformLocation(shader.second, "projection"), 1, 0, glm::value_ptr(*matrices.first));
		}
		for (Model model : scene.models) model.draw();

		//imgui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		//ImGui::DockSpaceOverViewport(); //allows dock to window but blocks rendering
		renderUI(); //my function
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//render multiple windows for imgui
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//shutdown procedure
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}