#pragma once
#include <glm/glm.hpp>

struct Tri {
	glm::vec3 col;
	glm::vec3 specularCol;
	glm::vec3 emissionCol;
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
	float IOR;
	float roughness;
	float emissionIntensity;
	float refraction;
	float metalness;
	uint32_t idx;
	bool doubleSided;

	Tri(glm::vec3 col, glm::vec3 specularCol, glm::vec3 emissionCol, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 aN, glm::vec3 bN, glm::vec3 cN,
		float IOR, float roughness, float emissionIntensity, float refraction, float metalness, bool doubleSided)
		: col(col), specularCol(specularCol), emissionCol(emissionCol), a(a), b(b), c(c), aN(aN), bN(bN), cN(cN), IOR(IOR),
		roughness(roughness), emissionIntensity(emissionIntensity), refraction(refraction), metalness(metalness), doubleSided(doubleSided) {
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