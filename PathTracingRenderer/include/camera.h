#pragma once
#include <raylib.h>
#include <iostream>

struct PTCam {

	glm::vec3 camPos = { 0.0f, -16.0f, 1.7f };
	glm::vec3 camTarget = { 0.0f, 0.0f, 1.7f };

	float targetDist = 1.0f;

	float camSpeed = 10.0f;
	float fov = 45.0f;

	glm::vec3 camNormal = { 0.0f, 1.0f, 0.0f };
	glm::vec3 worldUp = { 0.0f, 0.0f, 1.0f };
	glm::vec3 right = glm::normalize(glm::cross(camNormal, worldUp));
	glm::vec3 up = glm::normalize(glm::cross(right, camNormal));
	float halfFovRadians = glm::radians(fov) * 0.5f;
	float verticalScale = tan(halfFovRadians);

	glm::vec3 targetNormal = { 0.0f, 1.0f, 0.0f };

	bool keyDownSample(int key, Params& params) {
		if (IsKeyDown(key)) {
			params.shouldSample = false;
			params.enableSampling = false;
			return true;
		}
		else {
			params.enableSampling = true;
			return false;
		}
	}

	bool mouseDownSample(int key, Params& params) {
		if (IsMouseButtonDown(key)) {
			params.shouldSample = false;
			params.enableSampling = false;
			return true;
		}
		else {
			params.enableSampling = true;
			return false;
		}
	}

	float mouseSensitivity = 0.01f;

	void cameraLogic(Params& params) {
		float currentSpeed = camSpeed * params.dt;

		if (params.enableSampling) {
			params.shouldSample = true;
		}

		if (keyDownSample(KEY_A, params)) camPos -= right * currentSpeed;
		if (keyDownSample(KEY_D, params)) camPos += right * currentSpeed;
		if (keyDownSample(KEY_S, params)) camPos -= camNormal * currentSpeed;
		if (keyDownSample(KEY_W, params)) camPos += camNormal * currentSpeed;
		if (keyDownSample(KEY_LEFT_CONTROL, params)) camPos.z -= currentSpeed;
		if (keyDownSample(KEY_LEFT_SHIFT, params)) camPos.z += currentSpeed;

		if (mouseDownSample(1, params)) {
			glm::vec2 mDelta = { GetMouseDelta().x, GetMouseDelta().y };

			bool teleported = false;
			if (GetMousePosition().x < 0.0f) {
				SetMousePosition(params.screenSize.x, GetMousePosition().y);
				teleported = true;
			}
			if (GetMousePosition().x > params.screenSize.x) {
				SetMousePosition(0, GetMousePosition().y);
				teleported = true;
			}

			if (GetMousePosition().y < 0.0f) {
				SetMousePosition(GetMousePosition().x, params.screenSize.y);
				teleported = true;
			}
			if (GetMousePosition().y > params.screenSize.y) {
				SetMousePosition(GetMousePosition().x, 0);
				teleported = true;
			}

			if (!teleported) {
				glm::vec3 newTarget = camPos + targetNormal * 10.0f;

				float ratio = float(params.screenSize.x) / float(params.screenSize.y);

				if (mDelta.x != 0.0f) { 
					newTarget += right * mDelta.x * mouseSensitivity;
				}

				if (mDelta.y != 0.0f) { 
					newTarget -= up * mDelta.y * ratio * mouseSensitivity;
				}

				targetNormal = glm::normalize(newTarget - camPos);
			}
		}

		camTarget = camPos + targetNormal * 10.0f;

		halfFovRadians = glm::radians(fov) * 0.5f;
		verticalScale = tan(halfFovRadians);

		glm::vec3 camD = camTarget - camPos;

		camNormal = glm::normalize(camD);

		right = glm::normalize(glm::cross(camNormal, worldUp));
		up = glm::normalize(glm::cross(right, camNormal));
	}
};