#pragma once
#include <glm.hpp>

struct Gizmo {
	glm::vec3 pos;
	glm::vec3 worldUp = { 0.0f, 0.0f, 1.0f };
	glm::vec3 worldRight = { 1.0f, 0.0f, 0.0f };
	glm::vec3 worldForward = { 0.0f, 1.0f, 0.0f };
};