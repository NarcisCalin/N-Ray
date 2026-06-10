#pragma once

#include <glm/glm.hpp>
#include <raylib.h>
#include <tri.h>
#include <map>
#include <bvh.h>


struct PathRay;
struct PathRayState;
struct PTModel;

struct Data {
	std::vector<Tri> tris;
	std::vector<Tri> emTris;
	std::vector<PathRay> rays;
	std::vector<PathRayState> rayStates;
	std::vector<Color> frameBuffer;
	std::vector<glm::vec3> accumBuffer;
	std::vector<glm::vec3> causticsBuffer;
	std::vector<PTModel> models;

	std::vector<uint32_t> triMap;

	EmbreeBVH embreeBVH;
};
 
struct Params { 
	glm::ivec2 screenSize = { 1920, 1080 };

	float dt = 0.0f;

	int res = 512;

	int maxBounces = 5;
	int prevMaxBounces = maxBounces;
	int maxSamples = 50000;
	int raysPerPixel = 1;
	int currentSample = 0;
	bool enableSky = true;
	bool enableHDRI = true;
	float skyIntensity = 1.0f;
	float blur = 1.0f;
	float exposure = 1.0f;
	float contrast = 0.8f;
	float hdriThreshold = 0.0f;
	float hdriBrigthest = 0.0f;

	float hdriRotation = 0.0f;

	bool enableSun = false;
	glm::vec3 sunDir = { 0.0f, 0.0f, 1.0f };
	glm::vec3 sunColor = { 1.0f, 1.0f, 0.95f };
	float sunIntensity = 100.0f;
	float sunAngle = 7.53f;

	glm::vec3 skyTopOriginal = { 0.263f, 0.553f, 0.769f };
	glm::vec3 skyBaseOriginal = { 0.89f, 0.824f, 0.698f };

	glm::vec3 skyTop = skyTopOriginal;
	glm::vec3 skyBase = skyBaseOriginal;

	size_t refractiveAmount = 0;

	float totalMs = 0.0f;
	int totalFrames = 0;

	bool shouldSample = true;
	bool enableSampling = true;

	bool isMouseHoveringUI = false;
	bool enableDebugRay = false;
	bool enableSelection = true;
	bool render = true;

	bool enableCaustics = true;
	bool enableBiasCaustics = false;
	float causticsIntensity = 1.0f;
	float biasCausticsContactClamp = 0.1f;

	float mainBufferIntensity = 1.0f;

	float renderTime = 0.0f;

	bool rayMarcher = true;
	bool pathTracer = false;

	int rmMaxSteps = 1000;
	float rmNearPlane = 0.001f;
	float rmFarPlane = 1000.0f;

	glm::vec3 rmAlbedo = {0.7f, 0.7f, 0.7f};
	glm::vec3 rmSpecularCol = {1.0f, 1.0f, 1.0f};
	glm::vec3 rmEmissionCol = {0.0f, 0.0f, 0.0f};
	glm::vec3 rmAbsorptionCol = { 0.0f, 0.0f, 0.0f };
	glm::vec3 rmVolumeCol = { 0.0f, 0.0f, 0.0f };

	float rmIOR = 1.5f;
	float rmRoughness = 0.3f;
	float rmEmissionIntensity = 0.0f;
	float rmRefraction = 0.0f;
	float rmAbsorption = 0.0f;
	float rmVolume = 0.0f;
	float rmDensity = 0.0f;
	float rmMetalness = 0.0f;

	bool rmPBR = true;
};