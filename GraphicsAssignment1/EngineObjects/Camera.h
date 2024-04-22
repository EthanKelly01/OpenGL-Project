#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

class Camera {
public:
	Camera(float width, float height) : width(width), height(height) {};

	//glm::vec3 getPos() { return position; }

	void setPos(float posx, float posy, float posz, float yaw = -1, float pitch = -1) { 
		position = glm::vec3(posx, posy, posz);
		if (yaw != -1) this->yaw = yaw;
		if (pitch != -1) this->pitch = pitch;
	}

	void setVars(float speed = -1, float sensitivity = -1, float fov = -1) {
		if (speed != -1) this->speed = speed;
		if (sensitivity != -1) this->sensitivity = sensitivity;
		if (fov != -1) this->fov = fov;
	}

	void setPlanes(float near, float far) { this->near = near; this->far = far; }

	void setOrtho(bool ortho) { this->ortho = ortho; }

	void setTime(float deltaTime) { dtime = deltaTime; }
	
	void resizeWindow(float width, float height) {
		this->width = width;
		this->height = height;
		recompProj = 1;
	}

	std::pair<glm::mat4*, glm::mat4*>  getMatrices() {
		if (recompProj) {
			if (!ortho) projection = glm::perspective(fov, width / height, near, far);
			else projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, near, far);
		}
		if (recompView) view = glm::lookAt(position, position + front, up);

		recompView = recompProj = 0;
		return std::pair<glm::mat4*, glm::mat4*>(&projection, &view);
	}
	
	void debug() { 
		printf("Camera position: %f, %f, %f\n", position.x, position.y, position.z); 
		printf("Camera direction: %f, %f, %f\n\n", front.x, front.y, front.z);
	}

	virtual void processMouse(float xoffset, float yoffset, GLboolean contrainPitch = true) = 0;
	virtual void processScroll(float yoffset) = 0;
protected:
	glm::mat4 projection, view;
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
		front = glm::vec3(0.0f, 0.0f, -1.0f),
		up = glm::vec3(0.0f, 1.0f, 0.0f),
		right = glm::vec3(0.0f, 0.0f, 0.0f),
		worldUp = up;

	float fov = 45.0f, maxFOV = 70.0f, near = 0.1f, far = 100.0f, width, height,
		yaw = -90.0f, pitch = 0.0f, speed = 2.5f, sensitivity = 0.1f,
		dtime = 0;

	bool ortho = 0, recompView = 1, recompProj = 1;
};