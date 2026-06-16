#pragma once
#include <glm/glm.hpp>
#include <globalIds.h>
#include <material.h>
#include <raylib.h>

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
	uint32_t id;
	bool doubleSided;
	uint32_t modelId;

	Tri() = default;

	Tri(PTMaterial mat, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 aN, glm::vec3 bN, glm::vec3 cN, bool doubleSided)
		: mat(mat), a(a), b(b), c(c), aN(aN), bN(bN), cN(cN), doubleSided(doubleSided) {

		id = globalTriId++;
		initNormal();
		calculateAABB();
		calculateCenter();
	}

	void initNormal() {
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

	void drawNormals() {

		//glm::vec3 finalPos = center + normal * 0.1f;

		//Color col = { int(normal.x * 255.0f), int(normal.y * 255.0f), int(normal.z * 255.0f), 255 };
		//DrawLine3D({ center.x, center.y, center.z }, { finalPos.x, finalPos.y, finalPos.z }, col);

		glm::vec3 finalPosA = a + aN * 0.1f;

		Color colA = { int(aN.x * 255.0f), int(aN.y * 255.0f), int(aN.z * 255.0f), 255 };
		DrawLine3D({ a.x, a.y, a.z }, { finalPosA.x, finalPosA.y, finalPosA.z }, colA);

		glm::vec3 finalPosB = b + bN * 0.1f;

		Color colB = { int(bN.x * 255.0f), int(bN.y * 255.0f), int(bN.z * 255.0f), 255 };
		DrawLine3D({ b.x, b.y, b.z }, { finalPosB.x, finalPosB.y, finalPosB.z }, colB);

		glm::vec3 finalPosC = c + cN * 0.1f;

		Color colC = { int(cN.x * 255.0f), int(cN.y * 255.0f), int(cN.z * 255.0f), 255 };
		DrawLine3D({ c.x, c.y, c.z }, { finalPosC.x, finalPosC.y, finalPosC.z }, colC);
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