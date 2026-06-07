#pragma once
#include <glm/glm.hpp>
#include <globalParams.h>
#include <camera.h>
#include <bvh.h>
#include <screenStartup.h>
#include <iostream>
#include <rtcore.h>


struct PathRay {
	glm::vec3 src;
	glm::vec3 dir;
	glm::vec3 invDir;
};

struct PathRayState {
	glm::vec3 hitPos = { 0.0f, 0.0f, 0.0f };
	glm::vec3 col = { 0.0f, 0.0f, 0.0f };
	glm::vec3 throughput = { 1.0f, 1.0f, 1.0f };
	float length = FLT_MAX;
	uint32_t triIdx = UINT32_MAX;
	bool hit = false;
	bool active = true;
	bool isRefraction = false;
	bool isVolume = false;
};

struct DebugRay {
	glm::vec3 src;
	glm::vec3 dir;
	glm::vec3 col;
	float length;
	float progress = 0.0f;
};

struct Params;

struct PathTracer {

	bool RayIntersectsTriangle(PathRay& ray, const Tri& tri, float& t);

	bool rayAABB(const PathRay& ray, const glm::vec3& boxMin, const glm::vec3& boxMax, float maxT);

	void diffuseLighting(PathRay& ray, PathRayState& rayState, glm::vec3& normal, std::vector<Tri>& tris);

	const float airIOR = 1.0f;

	glm::vec3 sampleGGX(const glm::vec3& normal, float roughness, float r1, float r2);

	bool specularLighting(PathRay& ray, PathRayState& rayState, glm::vec3& normal, std::vector<Tri>& tris);

	void refractionLighting(PathRay& ray, PathRayState& rayState, glm::vec3 normal, std::vector<Tri>& tris);

	void flattenBVH(uint32_t buildNodeIdx, const std::vector<BVH>& buildNodes, std::vector<CompactBVH>& flatNodes);

	void traverseFlatBVH(PathRay& ray, PathRayState& rayState, float& closestT, std::vector<Tri>& tris, const std::vector<CompactBVH>& flatBVH);

	bool traceRay(const EmbreeBVH& bvh, const PathRay& ray, PathRayState& rayState, float& closestT);

	void directLight(glm::vec3& sampleP, glm::vec3& sampleN, float& pdf, Data& data, Params& params);

	glm::vec3 InterpolateNormal(PathRayState& rayState, std::vector<Tri>& tris);

	std::vector<DebugRay> debugRays;

	//void sampleSun(PathRay& ray, std::vector<Tri>& tris, Params& params, bool& isShadow); // CURRENTLY UNUSED

	std::vector<DebugRay> rayLogic(PathRay& ray, PathRayState& rayState, Params& params, Data& data, Image& hdri, bool debug = false);

	void rayGeneration(std::vector<PathRay>& rays, std::vector<PathRayState>& raysStates, PTCam& myCam, Screen& screen, Params& params);

	glm::vec3 contrastSCurve(glm::vec3 x, float c) {

		auto curve = [&](float v) -> float {
			if (v < 0.5f)
				return 0.5f * std::pow(2.0f * v, 1.0f + c);
			else
				return 1.0f - 0.5f * std::pow(2.0f * (1.0f - v), 1.0f + c);
			};
		return glm::clamp(glm::vec3(curve(x.r), curve(x.g), curve(x.b)), 0.0f, 1.0f);
	}

	void colorManagement(float c, glm::vec3& col) {
		col = contrastSCurve(col, c);
		col = glm::pow(col, glm::vec3(1.0f / 2.2f));
	}

	void drawScreen(Screen& screen, Params& params, Data& data, int& width, Texture2D& render);

	void render(Data& data, PTCam& myCam, Screen& screen, Params& params, Texture2D& render, Image& hdri);
};