#pragma once
#include <glm/glm.hpp>
#include <globalParams.h>
#include <camera.h>
#include <bvh.h>
#include <screenStartup.h>
#include <iostream>

struct PathRay {
	glm::vec3 src;
	glm::vec3 dir;
	glm::vec3 hitPos;
	glm::vec3 col = { 0.0f, 0.0f, 0.0f };
	glm::vec3 throughput = { 1.0f, 1.0f, 1.0f };
	uint32_t triIdx;
	bool hit = false;
	bool active = true;
	bool isRefraction = false;
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

	bool rayAABB(const PathRay& ray, glm::vec3& boxMin, glm::vec3& boxMax);

	void diffuseLighting(PathRay& ray, glm::vec3& normal, std::vector<Tri>& tris);

	const float airIOR = 1.0f;

	glm::vec3 sampleGGX(const glm::vec3& normal, float roughness, float r1, float r2);

	bool specularLighting(PathRay& ray, glm::vec3& normal, std::vector<Tri>& tris);

	void refractionLighting(PathRay& ray, glm::vec3 normal, std::vector<Tri>& tris);

	void traverseFlatBVH(PathRay& ray, float& closestT, std::vector<Tri>& tris);

	void directLight(PathRay& ray, glm::vec3 normal, std::vector<Tri>& tris, Params& params);

	glm::vec3 InterpolateNormal(PathRay& ray, std::vector<Tri>& tris) {

		glm::vec3 e0 = tris[ray.triIdx].b - tris[ray.triIdx].a;
		glm::vec3 e1 = tris[ray.triIdx].c - tris[ray.triIdx].a;
		glm::vec3 d = ray.hitPos - tris[ray.triIdx].a;

		float d00 = glm::dot(e0, e0);
		float d01 = glm::dot(e0, e1);
		float d11 = glm::dot(e1, e1);
		float d20 = glm::dot(d, e0);
		float d21 = glm::dot(d, e1);

		float denom = d00 * d11 - d01 * d01;

		float v = (d11 * d20 - d01 * d21) / denom;
		float w = (d00 * d21 - d01 * d20) / denom;
		float u = 1.0f - v - w;

		glm::vec3 interpolatedNormal =
			u * tris[ray.triIdx].aN +
			v * tris[ray.triIdx].bN +
			w * tris[ray.triIdx].cN;

		return glm::normalize(interpolatedNormal);
	}

	void rayLogic(PathRay& ray, std::vector<Tri>& tris, Params& params);

	void rayGeneration(std::vector<PathRay>& rays, PTCam& myCam, Screen& screen, Params& params);

	float contrast = 0.8f;

	bool shouldSample = true;

	void drawScreen(Screen& screen, Params& params) {

		float invSamples = params.currentSample > 0 ? 1.0f / float(params.currentSample) : 1.0f;

#pragma omp parallel for
		for (int i = 0; i < screen.pixels.size(); i++) {
			Pixel& p = screen.pixels[i];
			p.color = glm::min(p.accumColor * invSamples, 1.0f);
		}

		if (IsKeyPressed(KEY_UP)) {
			contrast += 0.1f;
			std::cout << "Contrast: " << contrast << '\n';
		}

		if (IsKeyPressed(KEY_DOWN)) {
			contrast -= 0.1f;
			std::cout << "Contrast: " << contrast << '\n';
		}

		for (int i = 0; i < screen.pixels.size(); i++) {
			Pixel& p = screen.pixels[i];

			p.colorManagement(contrast);
			p.draw();
		}
	}

	void render(std::vector<PathRay>& rays, std::vector<Tri>& tris, PTCam& myCam, Screen& screen, Params& params);

	std::vector<DebugRay> rayLogicDebug(PathRay ray, std::vector<Tri>& tris, Params& params);

};