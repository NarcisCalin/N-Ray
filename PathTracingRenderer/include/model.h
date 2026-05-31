#pragma once
#include <tri.h>
#include <vector>
#include <glm.hpp>
#include <cstdint>

struct Data;

struct PTModel {
	std::vector<uint32_t> tris;

	glm::vec3 albedo;
	glm::vec3 specularCol;
	glm::vec3 emissionCol;
	glm::vec3 refractionCol;

	float IOR;
	float roughness;
	float emissionIntensity;
	float refraction;
	float metalness;
	bool doubleSided;
	uint32_t idx;
	bool selected = false;

	PTModel(glm::vec3 albedo, glm::vec3 specularCol, glm::vec3 emissionCol, glm::vec3 refractionCol,
		float IOR, float roughness, float emissionIntensity, float refraction, float metalness, bool doubleSided, uint32_t idx)
		: albedo(albedo), specularCol(specularCol), emissionCol(emissionCol), refractionCol(refractionCol), IOR(IOR),
		roughness(roughness), emissionIntensity(emissionIntensity), refraction(refraction), metalness(metalness), doubleSided(doubleSided),
		idx(idx) {

	}

	void updateTris(Data& data);
};