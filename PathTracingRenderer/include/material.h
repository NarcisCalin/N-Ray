#pragma once
#include <glm/glm.hpp>

struct PTMaterial {
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

	PTMaterial(
		const glm::vec3& albedo,
		const glm::vec3& specularCol,
		const glm::vec3& emissionCol,
		const glm::vec3& absorptionCol,
		const glm::vec3& volumeCol,
		float IOR,
		float roughness,
		float emissionIntensity,
		float refraction,
		float absorption,
		float volume,
		float density,
		float metalness)
		:
		albedo(albedo),
		specularCol(specularCol),
		emissionCol(emissionCol),
		absorptionCol(absorptionCol),
		volumeCol(volumeCol),
		IOR(IOR),
		roughness(roughness),
		emissionIntensity(emissionIntensity),
		refraction(refraction),
		absorption(absorption),
		volume(volume),
		density(density),
		metalness(metalness)
	{
	}
};

struct PTMaterialGPU {
	glm::vec4 albedo_roughness;
	glm::vec4 specular_metalness;
	glm::vec4 emission_intensity;
	glm::vec4 absorption_amount;
	glm::vec4 volume_density;
	glm::vec4 extraParams; // x = IOR, y = refraction, z = volume, w = padding (unused)

	PTMaterialGPU() = default;

	PTMaterialGPU(
		const glm::vec3& albedo, float roughness,
		const glm::vec3& specularCol, float metalness,
		const glm::vec3& emissionCol, float emissionIntensity,
		const glm::vec3& absorptionCol, float absorption,
		const glm::vec3& volumeCol, float density,
		float IOR, float refraction, float volume
	) :
		albedo_roughness(glm::vec4(albedo, roughness)),
		specular_metalness(glm::vec4(specularCol, metalness)),
		emission_intensity(glm::vec4(emissionCol, emissionIntensity)),
		absorption_amount(glm::vec4(absorptionCol, absorption)),
		volume_density(glm::vec4(volumeCol, density)),
		extraParams(glm::vec4(IOR, refraction, volume, 0.0f))
	{
	}
};