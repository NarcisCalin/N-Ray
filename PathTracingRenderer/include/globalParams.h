#pragma once

#pragma once
#include <glm/glm.hpp>
#include <tri.h>
#include <raylib.h>


struct PathRay;

struct Data {
	std::vector<Tri> tris;
	std::vector<Tri> emTris;
	std::vector<PathRay> rays;
	std::vector<Color> frameBuffer;
	std::vector<glm::vec3> accumBuffer;
};
 
struct Params { 
	glm::ivec2 screenSize = { 1920, 1080 };

	float dt = 0.0f;

	int res = 512;

	int maxBounces = 5;
	int maxSamples = 50000;
	int currentSample = 0;
	bool environmentLight = true;
	float environmentIntensity = 0.75f;
	float blur = 0.5f;

	bool sunLight = true;
	glm::vec3 sunDir = { 0.0f, 0.0f, 1.0f };
	glm::vec3 sunColor = { 1.0f, 1.0f, 1.0f };
	float sunIntensity = 100.0f;
	float sunAngle = 7.53f;
	size_t emissiveAmount = 0;

	bool shouldSample = true;
	bool enableSampling = true;
};