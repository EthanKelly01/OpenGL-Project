#pragma once

#include "Camera.h"

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