#pragma once
#include <glm/glm.hpp>
#include <globalIds.h>
#include <material.h>

struct Tri {
	PTMaterial mat;
	glm::vec3 a; 
	glm::vec3 b;
	glm::vec3 c;
	glm::vec3 aN;
	glm::vec3 bN;
	glm::vec3 cN;
	glm::vec3 eA;
	glm::vec3 eB;
	glm::vec3 normal;
	glm::vec3 min;
	glm::vec3 max;
	glm::vec3 center;
	uint32_t id = globalTriId++;
	bool doubleSided;
	uint32_t modelId;

	Tri() = default;

	Tri(PTMaterial mat, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 aN, glm::vec3 bN, glm::vec3 cN, bool doubleSided)
		: mat(mat), a(a), b(b), c(c), aN(aN), bN(bN), cN(cN), doubleSided(doubleSided) {
		calculateNormal();
		calculateAABB();
		calculateCenter();
	}

	void calculateNormal() {
		eA = b - a;
		eB = c - a;

		normal = glm::normalize(glm::cross(eA, eB));
	}

	void calculateAABB() {
		min = glm::min(a, glm::min(b, c));
		max = glm::max(a, glm::max(b, c));
	}

	void calculateCenter() {
		center = (a + b + c) / 3.0f;
	}
};

struct TriGPU {
	PTMaterialGPU mat;

	glm::vec4 a;
	glm::vec4 b;
	glm::vec4 c;
	glm::vec4 aN;
	glm::vec4 bN;
	glm::vec4 cN;
	glm::vec4 eA;
	glm::vec4 eB;
	glm::vec4 normal;
	glm::vec4 min;
	glm::vec4 max;
	glm::vec4 center;

	uint32_t id;
	uint32_t doubleSided;
	uint32_t modelId;
	uint32_t padding;
};