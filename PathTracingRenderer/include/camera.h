#pragma once
#include <raylib.h>

struct PTCam {

	glm::vec3 camPos = { 0.0f, -16.0f, 1.7f };
	glm::vec3 camTarget = { 0.0f, 0.0f, 1.7f };

	float camSpeed = 10.0f;
	float fov = 45.0f;

	glm::vec3 camNormal = { 0.0f, 1.0f, 0.0f };
	glm::vec3 worldUp = { 0.0f, 0.0f, 1.0f };
	glm::vec3 right = glm::normalize(glm::cross(camNormal, worldUp));
	glm::vec3 up = glm::normalize(glm::cross(right, camNormal));
	float halfFovRadians = glm::radians(fov) * 0.5f;
	float verticalScale = tan(halfFovRadians);

	void cameraLogic(float& dt) {
		float currentSpeed = camSpeed * dt;

		if (IsKeyDown(KEY_A)) camPos.x -= currentSpeed;
		if (IsKeyDown(KEY_D)) camPos.x += currentSpeed;
		if (IsKeyDown(KEY_S)) camPos.y -= currentSpeed;
		if (IsKeyDown(KEY_W)) camPos.y += currentSpeed;
		if (IsKeyDown(KEY_LEFT_CONTROL)) camPos.z -= currentSpeed;
		if (IsKeyDown(KEY_LEFT_SHIFT)) camPos.z += currentSpeed;

		halfFovRadians = glm::radians(fov) * 0.5f;
		verticalScale = tan(halfFovRadians);

		glm::vec3 camD = camTarget - camPos;

		camNormal = glm::normalize(camD);

		right = glm::normalize(glm::cross(camNormal, worldUp));
		up = glm::normalize(glm::cross(right, camNormal));
	}
};