#pragma once
#include <glm.hpp>
#include <io.h>

struct ObjectManipulation {

	bool grab = false;
	bool rotate = false;
	bool scale = false;

	bool setAxis = false;

	glm::vec3 axis = { 0.0f, 0.0f, 0.0f };

	float grabSpeed = 0.01f;
	float rotateSpeed = 1.0f;
	float scaleSpeed = 0.01f;

	bool discardChange = false;

	bool applyChange = false;

	bool storeInitData = false;

	bool hasStartedTransform = false;

	void initTransform(uint32_t& selectedModelsAmount) {

		if ((IO::mousePress(0) || IO::mousePress(1)) && (grab || rotate || scale)) {
			grab = false;
			rotate = false;
			scale = false;

			setAxis = false;

			if (selectedModelsAmount > 0) {
				if (IO::mousePress(0)) {
					applyChange = true;
					hasStartedTransform = false;
				}

				if (IO::mousePress(1)) {
					discardChange = true;
					hasStartedTransform = false;
				}
			}
		}

		if (selectedModelsAmount > 0) {
			if (IO::shortcutPress(KEY_E)) {
				grab = true;
				rotate = false;
				scale = false;

				setAxis = false;

				if (!hasStartedTransform) {
					storeInitData = true;
				}

				hasStartedTransform = true;
			}

			if (IO::shortcutPress(KEY_R)) {
				grab = false;
				rotate = true;
				scale = false;

				setAxis = false;

				if (!hasStartedTransform) {
					storeInitData = true;
				}

				hasStartedTransform = true;
			}

			if (IO::shortcutPress(KEY_T)) {
				grab = false;
				rotate = false;
				scale = true;

				setAxis = false;

				if (!hasStartedTransform) {
					storeInitData = true;
				}

				hasStartedTransform = true;
			}
		}

		if (grab || rotate || scale) {
			if (IO::shortcutPress(KEY_X)) {
				setAxis = true;

				axis = { 1.0f, 0.0f, 0.0f };
			}

			if (IO::shortcutPress(KEY_Y)) {
				setAxis = true;

				axis = { 0.0f, 1.0f, 0.0f };
			}

			if (IO::shortcutPress(KEY_Z)) {
				setAxis = true;

				axis = { 0.0f, 0.0f, 1.0f };
			}
		}
	}

	glm::vec3 grabLogic(glm::vec3& right, glm::vec3& up, bool& hasSceneChanged) {

		glm::vec3 moveChange = { 0.0f, 0.0f, 0.0f };

		glm::vec2 delta = { GetMouseDelta().x, GetMouseDelta().y };

		if (!setAxis) {
			moveChange += delta.x * right * grabSpeed;
			moveChange -= delta.y * up * grabSpeed;
		}
		else {

			if (axis.z == 0.0f) {
				moveChange += delta.x * axis * grabSpeed;
			}
			else {
				moveChange -= delta.y * axis * grabSpeed;
			}
		}

		if (moveChange.x != 0.0f || moveChange.y != 0.0f || moveChange.z != 0.0f) {
			hasSceneChanged = true;
		}

		int screenWidth = GetScreenWidth();
		int screenHeight = GetScreenHeight();

		SetMousePosition(screenWidth / 2, screenHeight / 2);

		return moveChange;
	}

	glm::vec4 rotateLogic(glm::vec3& right, glm::vec3& up, bool& hasSceneChanged) {

		glm::vec3 relAxis = { 0.0f, 0.0f, 0.0f };
		float rotation = 0.0f;

		glm::vec2 delta = { GetMouseDelta().x, GetMouseDelta().y };

		if (!setAxis) {
			relAxis += right;
			relAxis += up;
		}
		else {
			relAxis = axis;
		}

		relAxis = glm::normalize(relAxis);

		rotation += delta.x * rotateSpeed;

		if (rotation != 0.0f) {
			hasSceneChanged = true;
		}

		int screenWidth = GetScreenWidth();
		int screenHeight = GetScreenHeight();

		SetMousePosition(screenWidth / 2, screenHeight / 2);

		glm::vec4 rotationAndAxis = { relAxis, rotation };

		return rotationAndAxis;
	}

	glm::vec4 scaleLogic(glm::vec3& right, glm::vec3& up, bool& hasSceneChanged) {

		glm::vec3 relAxis = { 0.0f, 0.0f, 0.0f };
		float scale = 0.0f;

		glm::vec2 delta = { GetMouseDelta().x, GetMouseDelta().y };

		if (!setAxis) {
			relAxis = { 1.0f, 1.0f, 1.0f };
		}
		else {
			relAxis = axis;
		}

		relAxis = glm::normalize(relAxis);

		scale += delta.x * scaleSpeed;

		if (scale != 0.0f) {
			hasSceneChanged = true;
		}

		int screenWidth = GetScreenWidth();
		int screenHeight = GetScreenHeight();

		SetMousePosition(screenWidth / 2, screenHeight / 2);

		glm::vec4 scaleAndAxis = { relAxis, scale };

		return scaleAndAxis;
	}
};