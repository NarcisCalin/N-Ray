#include "model.h"
#include "globalParams.h"

void PTModel::updateTris(Data& data) {
	for (size_t i = 0; i < tris.size(); i++) {

		Tri& t = data.tris[tris[i]];

		t.mat = mat;

		t.doubleSided = doubleSided;
	}
}

void PTModel::moveModel(ObjectManipulation& objMan, Data& data, bool& hasSceneChanged, glm::vec3& right, glm::vec3& up) {

	glm::vec3 delta = objMan.grabLogic(right, up, hasSceneChanged);

	for (size_t i = 0; i < tris.size(); i++) {

		Tri& tri = data.tris[tris[i]];

		tri.a += delta;
		tri.b += delta;
		tri.c += delta;
	}

	recalculateTriData(data);
}

void PTModel::rotateModel(ObjectManipulation& objMan, Data& data, bool& hasSceneChanged, glm::vec3& right, glm::vec3& up) {

	glm::vec4 rotationAndAxis = objMan.rotateLogic(right, up, hasSceneChanged);

	glm::mat4 rot = glm::translate(glm::mat4(1.0f), origin) *
		glm::rotate(glm::mat4(1.0f),
			glm::radians(rotationAndAxis.w),
			{ rotationAndAxis.x, rotationAndAxis.y, rotationAndAxis.z }) *
		glm::translate(glm::mat4(1.0f), -origin);

	auto rotateVec = [&](glm::vec3& v)
		{
			v = glm::vec3(rot * glm::vec4(v, 1.0f));
		};

	auto rotateNormal = [&](glm::vec3& n)
		{
			n = glm::normalize(
				glm::vec3(rot * glm::vec4(n, 0.0f))
			);
		};

	for (size_t i = 0; i < tris.size(); i++) {

		uint32_t idx = tris[i];

		rotateVec(data.tris[idx].a);
		rotateVec(data.tris[idx].b);
		rotateVec(data.tris[idx].c);

		rotateNormal(data.tris[idx].aN);
		rotateNormal(data.tris[idx].bN);
		rotateNormal(data.tris[idx].cN);

		rotateNormal(data.tris[idx].normal);
	}

	recalculateTriData(data);
}

void PTModel::scaleModel(ObjectManipulation& objMan, Data& data, bool& hasSceneChanged, glm::vec3& right, glm::vec3& up) {

	glm::vec4 scaleAndAxis = objMan.scaleLogic(right, up, hasSceneChanged);

	glm::vec3 axis = { scaleAndAxis.x, scaleAndAxis.y, scaleAndAxis.z };

	for (size_t i = 0; i < tris.size(); i++) {

		Tri& tri = data.tris[tris[i]];

		glm::vec3 offsetA = originalOrigin - originalTris[i].a;
		glm::vec3 offsetB = originalOrigin - originalTris[i].b;
		glm::vec3 offsetC = originalOrigin - originalTris[i].c;

		tri.a -= axis * scaleAndAxis.w * offsetA;
		tri.b -= axis * scaleAndAxis.w * offsetB;
		tri.c -= axis * scaleAndAxis.w * offsetC;
	}

	recalculateTriData(data);
}

void PTModel::transformLogic(ObjectManipulation& objMan, Data& data, bool& hasSceneChanged, glm::vec3& right, glm::vec3& up) {

	if (objMan.storeInitData) {
		
		originalTris.resize(tris.size());

#pragma omp parallel for
		for (size_t i = 0; i < tris.size(); i++) {
			originalTris[i] = data.tris[tris[i]];
		}

		originalOrigin = origin;
	}

	if (objMan.grab) {
		moveModel(objMan, data, hasSceneChanged, right, up);

		objMan.storeInitData = false;
	}

	if (objMan.rotate) {
		rotateModel(objMan, data, hasSceneChanged, right, up);

		objMan.storeInitData = false;
	}

	if (objMan.scale) {
		scaleModel(objMan, data, hasSceneChanged, right, up);

		objMan.storeInitData = false;
	}

	if (objMan.discardChange) {
		
#pragma omp parallel for
		for (size_t i = 0; i < tris.size(); i++) {
			data.tris[tris[i]] = originalTris[i];
		}

		origin = originalOrigin;

		objMan.discardChange = false;
		hasSceneChanged = true;
	}

	if (objMan.applyChange) {
		calculateOriginCenter(data);
		objMan.applyChange = false;
		hasSceneChanged = true;
	}
}

void PTModel::recalculateTriData(Data& data) {

	for (size_t i = 0; i < tris.size(); i++) {

		uint32_t idx = tris[i];

		data.tris[idx].calculateAABB();
		data.tris[idx].calculateCenter();
	}
}

void PTModel::calculateOriginCenter(Data& data) {

	origin = { 0.0f, 0.0f, 0.0f };

	for (size_t i = 0; i < tris.size(); i++) {
		origin += data.tris[tris[i]].center;
	}

	origin /= float(tris.size());
}
