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

//free 3d flight
class FlyCam : public Camera {
public:
	FlyCam(float width, float height) : Camera(width, height) {}

	void setSprint() { sprint = 1; }

	void setMovement(std::vector<Camera_Movement> direction) { movement = direction; }

	void processMouse(float xoffset, float yoffset, GLboolean contrainPitch = true) {
		yaw += xoffset * sensitivity;
		pitch += yoffset * sensitivity;

		if (contrainPitch) {
			if (pitch > 89.0f) pitch = 89.0f;
			else if (pitch < -89.0f) pitch = -89.0f;
		}

		front = glm::normalize(glm::vec3(cos(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(pitch)), sin(glm::radians(yaw)) * cos(glm::radians(pitch))));
		right = glm::normalize(glm::cross(front, worldUp));
		up = glm::normalize(glm::cross(right, front));

		recompView = 1;
	}

	void processScroll(float yoffset) {
		fov -= yoffset;
		if (fov < 1.0f) fov = 1.0f;
		else if (fov > maxFOV) fov = maxFOV;
		recompProj = 1;
	}

	std::pair<glm::mat4*, glm::mat4*>  getMatrices() {
		if (movement.size() != 0) {
			recompView = 1;
			glm::vec3 output(0.0f);
			for (int i = 0; i < movement.size(); i++) {
				switch (movement[i]) {
				case FORWARD:
					output += front;
					break;
				case BACKWARD:
					output -= front;
					break;
				case LEFT:
					output -= right;
					break;
				case RIGHT:
					output += right;
					break;
				case UP:
					output += up;
					break;
				case DOWN:
					output -= up;
				}
			}
			if (sprint) position += output * (speed * dtime * 3 / movement.size());
			else position += output * (speed * dtime / movement.size());
			movement.clear();
		}

		if (recompProj) {
			if (!ortho) projection = glm::perspective(fov, width / height, near, far);
			else projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, near, far);
		}
		if (recompView) view = glm::lookAt(position, position + front, up);

		recompView = recompProj = sprint = 0;
		return std::pair<glm::mat4*, glm::mat4*>(&projection, &view);
	}

private:
	std::vector<Camera_Movement> movement;
	bool sprint = 0;
};