#pragma once
#include <tri.h>
#include <vector>
#include <glm.hpp>
#include <cstdint>
#include <globalIds.h>
#include <material.h>
#include <glm/gtc/matrix_transform.hpp>
#include <objectManipulation.h>
#include <io.h>

struct Data;
struct Params;

struct PTModel {
	std::vector<uint32_t> tris;

	PTMaterial mat;
	glm::vec3 origin;
	uint32_t id = globalModelId++;
	bool doubleSided = false;
	bool selected = false;

	std::vector<Tri> originalTris;
	glm::vec3 originalOrigin = origin;

	PTModel() = default;

	PTModel(PTMaterial mat, bool doubleSided)
		:  mat(mat), doubleSided(doubleSided) {
	}

	void updateTris(Data& data);

	void moveModel(ObjectManipulation& objMan, Data& data, bool& hasSceneChanged, glm::vec3& right, glm::vec3& up);

	void rotateModel(ObjectManipulation& objMan, Data& data, bool& hasSceneChanged, glm::vec3& right, glm::vec3& up);

	void scaleModel(ObjectManipulation& objMan, Data& data, bool& hasSceneChanged, glm::vec3& right, glm::vec3& up);

	void transformLogic(ObjectManipulation& objMan, Data& data, bool& hasSceneChanged, glm::vec3& right, glm::vec3& up);

	void recalculateTriData(Data& data);

	void calculateOriginCenter(Data& data);
};