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
	glm::vec3 invDir;
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

	std::vector<DebugRay> debugRays;

	void sampleSun(PathRay& ray, std::vector<Tri>& tris, Params& params, float& isShadow); // CURRENTLY UNUSED

	std::vector<DebugRay> rayLogic(PathRay& ray, std::vector<Tri>& tris, Params& params, bool debug = false);

	void rayGeneration(std::vector<PathRay>& rays, PTCam& myCam, Screen& screen, Params& params);

	float contrast = 0.8f;

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

	void drawScreen(Screen& screen, Params& params, Data& data, int& width, Texture2D& render) {

		float invSamples = params.currentSample > 0 ? 1.0f / float(params.currentSample) : 1.0f;

#pragma omp parallel for
		for (int i = 0; i < data.frameBuffer.size(); i++) {

			glm::vec3 col = glm::min(data.accumBuffer[i] * invSamples, 1.0f);

			colorManagement(contrast, col);

			Color finalCol = { unsigned char(col.x * 255),
				unsigned char(col.y * 255),
				unsigned char(col.z * 255), 255 };

			data.frameBuffer[i] = finalCol;
		}

		UpdateTexture(render, data.frameBuffer.data());
		DrawTexturePro(
			render,
			{ 0, 0, (float)screen.resX, (float)screen.resY },
			{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() },
			{ 0, 0 },
			0.0f,
			WHITE
		);

		if (IsKeyPressed(KEY_UP)) {
			contrast += 0.1f;
			std::cout << "Contrast: " << contrast << '\n';
		}

		if (IsKeyPressed(KEY_DOWN)) {
			contrast -= 0.1f;
			std::cout << "Contrast: " << contrast << '\n';
		}
	}

	void render(Data& data, PTCam& myCam, Screen& screen, Params& params, Texture2D& render);
};