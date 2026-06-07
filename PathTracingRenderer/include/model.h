#pragma once
#include <tri.h>
#include <vector>
#include <glm.hpp>
#include <cstdint>
#include <globalIds.h>

struct Data;

struct PTModel {
	std::vector<uint32_t> tris;

	glm::vec3 albedo;
	glm::vec3 specularCol;
	glm::vec3 emissionCol;
	glm::vec3 absorptionCol;
	glm::vec3 volumeCol;

	float IOR;
	float roughness;
	float emissionIntensity;
	float refraction;
	float absorption;
	float volume;
	float density;
	float metalness;
	bool doubleSided;
	uint32_t id = globalModelId++;
	bool selected = false;

	PTModel() = default;

	PTModel(glm::vec3 albedo, glm::vec3 specularCol, glm::vec3 emissionCol, glm::vec3 absorptionCol, glm::vec3 volumeCol,
		float IOR, float roughness, float emissionIntensity, float refraction, float absorption, float volume, float density, float metalness, bool doubleSided)
		: albedo(albedo), specularCol(specularCol), emissionCol(emissionCol), absorptionCol(absorptionCol), volumeCol(volumeCol), IOR(IOR),
		roughness(roughness), emissionIntensity(emissionIntensity), refraction(refraction), absorption(absorption), volume(volume), density(density), metalness(metalness), doubleSided(doubleSided) {
	}

	void updateTris(Data& data);
};