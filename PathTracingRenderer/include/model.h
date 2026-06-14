#pragma once
#include <tri.h>
#include <vector>
#include <glm.hpp>
#include <cstdint>
#include <globalIds.h>
#include <material.h>

struct Data;

struct PTModel {
	std::vector<uint32_t> tris;

	PTMaterial mat;
	bool doubleSided = false;
	uint32_t id = globalModelId++;
	bool selected = false;

	PTModel() = default;

	PTModel(PTMaterial mat, bool doubleSided)
		:  mat(mat), doubleSided(doubleSided) {
	}

	void updateTris(Data& data);
};