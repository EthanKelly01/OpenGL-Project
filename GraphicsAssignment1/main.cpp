#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "ModelCode/Shaders.h"
#include "ModelCode/Textures.h"
#include "ModelCode/Model.h"

#include "EngineObjects/FlyCam.h"

float windowWidth = 800.0f, windowHeight = 600.0f; //startup size

//TODO: ECS?
std::vector<GLuint> shaders; //scene objects
std::vector<GLuint> textures;
std::vector<Model> models;
FlyCam camera(windowWidth, windowHeight);

float deltaTime = 0.0f, lastFrame = 0.0f; //timing

bool menu = 0, prevEsc = 0, firstMouse = 1, lastDebug = 0; //input stuff
float lastX = windowWidth / 2, lastY = windowHeight / 2;

const std::string vs = "shaders/myShader.vs", fs = "shaders/myShader.fs";
//const std::string vs = "shaders/lightShader.vs", fs = "shaders/lightShader.fs";
const std::string containerTex = "assets/container.jpg", faceTex = "assets/awesomeface.png";
const std::string obj1 = "assets/Vase.obj";

static void resizeWindow(GLFWwindow* window, int width, int height) { 
	glViewport(0, 0, width, height);
	camera.resizeWindow(windowWidth = width, windowHeight = height);
}

static void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
		if (!lastDebug) {
			camera.debug();
			for (Model model : models) model.debug();
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
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "LearnOpenGL", NULL, NULL);
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

	//build objects
	shaders.push_back(buildProgram(vs, fs, 0));

	textures.push_back(buildTexture(containerTex));
	textures.push_back(buildTexture(faceTex));

	models.push_back(Model(obj1, shaders[0]));
	models[0].addTexture({ textures[0], "" });
	models[0].addTexture({ textures[1], "" });
	models[0].createInstance(glm::vec3(0, 0, 0), glm::vec3(10, 10, 10), glm::vec3(0, 5, 0));
	models[0].createInstance(glm::vec3(10, 0, 0), glm::vec3(0, -1.0, 0), glm::vec3(0, 5, 0));

	while (!glfwWindowShouldClose(window)) { //update loop
		deltaTime = glfwGetTime() - lastFrame;
		lastFrame = glfwGetTime();

		processInput(window);
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //just clears to some colour

		std::pair<glm::mat4*, glm::mat4*> matrices = camera.getMatrices(); //applying matrices to shaders
		//glm::vec3 eye = camera.getPos();
		//glm::mat3 normal = glm::transpose(glm::inverse(glm::mat3(*matrices.second)));
		for (GLuint shader : shaders) {
			glUniform3f(glGetUniformLocation(shader, "lightPos"), 30.0, 0.0, 0.0);

			glUseProgram(shader);
			glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(*matrices.first));
			glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(*matrices.second));

			//lighting stuff
			//glUniformMatrix3fv(glGetUniformLocation(shader, "normal"), 1, GL_FALSE, glm::value_ptr(normal));
			//glUniform3f(glGetUniformLocation(shader, "eye"), eye.x, eye.y, eye.z);
			//glUniform4f(glGetUniformLocation(shader, "colour"), 0.3f, 0.7f, 1.0f, 1.0f);
		}
		for (Model model : models) model.draw();
		glfwSwapBuffers(window);

		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}