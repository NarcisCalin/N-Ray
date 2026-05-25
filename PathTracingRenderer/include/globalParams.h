#pragma once

#pragma once
#include <glm/glm.hpp>
#include <tri.h>


struct PathRay;

struct Data {
	std::vector<Tri> tris;
	std::vector<Tri> emTris;
	std::vector<PathRay> rays;
};
 
struct Params { 
	glm::ivec2 screenSize = { 960, 1080 };

	float dt = 0.0f;

	int maxBounces = 5;
	int maxSamples = 50000;
	int currentSample = 0;
	bool environmentLight = true;
	float environmentIntensity = 0.0f;
	float blur = 0.5f;

	bool sunLight = false;
	glm::vec3 sunDir = { 0.0f, 0.0f, 1.0f };
	glm::vec3 sunColor = { 1.0f, 1.0f, 1.0f };
	float sunIntensity = 30.0f;
	float sunAngle = 5.5f;
	float sunCosAngle = cos(glm::radians(sunAngle));
	size_t emissiveAmount = 0;
};