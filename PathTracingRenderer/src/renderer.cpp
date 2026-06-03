#include "renderer.h"
#include <random>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <omp.h>

bool PathTracer::RayIntersectsTriangle(PathRay& ray, const Tri& tri, float& t) {
	const float EPSILON = 0.0000001f;

	glm::vec3 edge1 = tri.b - tri.a;
	glm::vec3 edge2 = tri.c - tri.a;

	glm::vec3 h = glm::cross(ray.dir, edge2);
	float det = glm::dot(edge1, h);

	if (tri.doubleSided) {
		if (det > -EPSILON && det < EPSILON) {
			return false;
		}
	}
	else {
		if (det < EPSILON) {
			return false;
		}
	}

	float invDet = 1.0f / det;

	glm::vec3 s = ray.src - tri.a;
	float u = invDet * glm::dot(s, h);

	if (u < 0.0f || u > 1.0f) {
		return false;
	}

	glm::vec3 q = glm::cross(s, edge1);
	float v = invDet * glm::dot(ray.dir, q);

	if (v < 0.0f || u + v > 1.0f) {
		return false;
	}

	t = invDet * glm::dot(edge2, q);

	if (t > EPSILON) {
		return true;
	}

	return false;
}

bool PathTracer::rayAABB(const PathRay& ray, const glm::vec3& boxMin, const glm::vec3& boxMax, float maxT) {
	float tx1 = (boxMin.x - ray.src.x) * ray.invDir.x;
	float tx2 = (boxMax.x - ray.src.x) * ray.invDir.x;

	float tmin = std::min(tx1, tx2);
	float tmax = std::max(tx1, tx2);

	float ty1 = (boxMin.y - ray.src.y) * ray.invDir.y;
	float ty2 = (boxMax.y - ray.src.y) * ray.invDir.y;

	tmin = std::max(tmin, std::min(ty1, ty2));
	tmax = std::min(tmax, std::max(ty1, ty2));

	float tz1 = (boxMin.z - ray.src.z) * ray.invDir.z;
	float tz2 = (boxMax.z - ray.src.z) * ray.invDir.z;

	tmin = std::max(tmin, std::min(tz1, tz2));
	tmax = std::min(tmax, std::max(tz1, tz2));

	return tmax >= std::max(tmin, 0.0f) && tmin < maxT;
}

void PathTracer::diffuseLighting(PathRay& ray, PathRayState& rayState, glm::vec3& normal, std::vector<Tri>& tris) {

	static thread_local std::mt19937 rng(std::random_device{}());
	static thread_local std::uniform_real_distribution<float> dist(0.0f, 1.0f);

	float u1 = dist(rng);
	float u2 = dist(rng);
	float r = sqrtf(u1);
	float phi = 2.0f * PI * u2;

	glm::vec3 localDir(
		r * cosf(phi),
		r * sinf(phi),
		sqrtf(1.0f - u1)
	);

	glm::vec3 up = fabsf(normal.z) < 0.999f ? glm::vec3(0, 0, 1) : glm::vec3(1, 0, 0);
	glm::vec3 tangent = glm::normalize(glm::cross(up, normal));
	glm::vec3 bitangent = glm::cross(normal, tangent);

	glm::vec3 worldDir =
		localDir.x * tangent +
		localDir.y * bitangent +
		localDir.z * normal;

	worldDir = glm::normalize(worldDir);

	if (glm::dot(worldDir, tris[rayState.triIdx].normal) < 0.0f) {
		worldDir = -worldDir;
	}

	ray.dir = worldDir;
}

glm::vec3 PathTracer::sampleGGX(const glm::vec3& normal, float roughness, float r1, float r2) {
	float a = roughness * roughness;

	float theta = atan(a * sqrt(r1) / sqrt(1.0f - r1));
	float phi = 2.0f * PI * r2;

	glm::vec3 h = glm::vec3(
		sin(theta) * cos(phi),
		sin(theta) * sin(phi),
		cos(theta)
	);

	glm::vec3 up = fabs(normal.z) < 0.999f ? glm::vec3(0, 0, 1) : glm::vec3(1, 0, 0);
	glm::vec3 tangent = glm::normalize(glm::cross(up, normal));
	glm::vec3 bitangent = glm::cross(normal, tangent);

	return glm::normalize(tangent * h.x + bitangent * h.y + normal * h.z);
}

bool PathTracer::specularLighting(PathRay& ray, PathRayState& rayState, glm::vec3& normal, std::vector<Tri>& tris) {
	static thread_local std::mt19937 rng(std::random_device{}());
	static thread_local std::uniform_real_distribution<float> dist(0.0f, 1.0f);

	float r1 = dist(rng);
	float r2 = dist(rng);
	glm::vec3 microfacetNormal = sampleGGX(normal, tris[rayState.triIdx].roughness, r1, r2);

	float r0Dielectric = ((airIOR - tris[rayState.triIdx].IOR) / (airIOR + tris[rayState.triIdx].IOR));
	r0Dielectric = r0Dielectric * r0Dielectric;

	float r0 = glm::mix(r0Dielectric, 1.0f, tris[rayState.triIdx].metalness);

	float cosThetaI = glm::clamp(-glm::dot(ray.dir, microfacetNormal), 0.0f, 1.0f);
	float rTheta = r0 + (1.0f - r0) * std::pow(1.0f - cosThetaI, 5.0f);

	if (dist(rng) > rTheta) {
		return false;
	}

	glm::vec3 reflectedDir = glm::reflect(ray.dir, microfacetNormal);

	if (glm::dot(reflectedDir, normal) <= 0.0f ||
		glm::dot(reflectedDir, tris[rayState.triIdx].normal) <= 0.0f) {
		return false;
	}

	ray.dir = reflectedDir;
	return true;
}

void PathTracer::refractionLighting(PathRay& ray, PathRayState& rayState, glm::vec3 normal, std::vector<Tri>& tris) {
	float n1 = airIOR;
	float n2 = tris[rayState.triIdx].IOR;
	float cosi = glm::dot(ray.dir, normal);

	if (cosi > 0.0f) {
		std::swap(n1, n2);
		normal = -normal;
		cosi = -cosi;
		rayState.isRefraction = false;
	}
	else {
		cosi = -cosi;
		ray.src = rayState.hitPos - normal * 0.001f;
		rayState.isRefraction = true;
		rayState.throughput *= tris[rayState.triIdx].refractionCol;
	}

	float eta = n1 / n2;
	float k = 1.0f - eta * eta * (1.0f - cosi * cosi);

	if (k < 0.0f) {
		ray.dir = glm::reflect(ray.dir, normal);
		return;
	}

	ray.dir = glm::normalize(glm::refract(ray.dir, normal, eta));
}

void PathTracer::traverseFlatBVH(PathRay& ray, PathRayState& rayState, float& closestT, const std::vector<Tri>& tris, const std::vector<CompactBVH>& flatBVH) {

	uint32_t idx = 0;
	uint32_t nodeCount = static_cast<uint32_t>(flatBVH.size());

	if (nodeCount == 0) return;

	while (idx < nodeCount) {
		const CompactBVH& node = flatBVH[idx];

		if (!rayAABB(ray, node.min, node.max, closestT)) {

			if (node.triCount > 0) {
				idx++;
			}
			else {
				idx = node.missLink;
			}
			continue;
		}

		if (node.triCount > 0) {

			for (uint32_t i = 0; i < node.triCount; ++i) {

				float t;
				const Tri& tri = tris[node.startIndex + i];

				if (RayIntersectsTriangle(ray, tri, t)) {
					if (t < closestT) {
						closestT = t;
						rayState.hit = true;
						rayState.hitPos = ray.src + ray.dir * t;
						rayState.triIdx = tri.idx;
					}
				}
			}

			idx++;

		}
		else {
			idx++;
		}
	}
}

//void PathTracer::directLight(PathRay& ray, glm::vec3 normal, std::vector<Tri>& tris, Params& params) {
//
//	static thread_local std::mt19937 rng(std::random_device{}());
//	thread_local std::uniform_int_distribution<size_t> idx(0, params.emissiveAmount - 1);
//	thread_local std::uniform_real_distribution<float> uv(0.0f, 1.0f);
//
//	size_t emIdx = idx(rng);
//
//	float u = uv(rng);
//	float v = uv(rng);
//
//	if (u + v > 1.0f) {
//		u = 1.0f - u;
//		v = 1.0f - v;
//	}
//
//	glm::vec3 sampleP = u * tris[emIdx].a + v * tris[emIdx].b + (1.0f - u - v) * tris[emIdx].c;
//	glm::vec3 sampleN = tris[emIdx].normal;
//
//	glm::vec3 lVec = sampleP - ray.hitPos;
//	float dSq = glm::dot(lVec, lVec);
//	float dist = sqrt(dSq);
//	glm::vec3 dlDir = lVec / dist;
//
//	PathRay dlRay({ ray.hitPos + tris[ray.triIdx].normal * 0.001f, dlDir });
//
//	float closestT = FLT_MAX;
//	traverseFlatBVH(dlRay, closestT, tris, globalCompactBVH);
//
//	if (closestT < dist - 0.001f) {
//		return;
//	}
//
//	glm::vec3 ab = tris[emIdx].a - tris[emIdx].b;
//	glm::vec3 ac = tris[emIdx].a - tris[emIdx].c;
//
//	float area = 0.5f * glm::length(glm::cross(ab, ac));
//
//	float pdf = 1.0f / (float(params.emissiveAmount) * area);
//
//	glm::vec3 dlDirT = glm::normalize(sampleP - ray.hitPos);
//	glm::vec3 diff = sampleP - ray.hitPos;
//	float distSq = glm::dot(diff, diff);
//
//	float cosSurface = glm::max(glm::dot(normal, dlDirT), 0.0f);
//	float cosLight = glm::max(glm::dot(sampleN, -dlDirT), 0.0f);
//
//	float G = (cosSurface * cosLight) / distSq;
//
//	glm::vec3 emission = tris[emIdx].emissionCol * tris[emIdx].emissionIntensity;
//
//	ray.col += ray.throughput * emission * G / (pdf * PI);
//}

glm::vec3 sky(PathRay& ray, Params& params) {

	glm::vec3 skyCol(params.skyIntensity);

	if (params.enableSky) {
		glm::vec3 worldUp = { 0.0f, 0.0f, 1.0f };

		glm::vec3 skyTop = { 0.263f, 0.553f, 0.769f };
		glm::vec3 skyBase = { 0.89f, 0.824f, 0.698f };

		float upAmount = glm::pow(glm::max(glm::dot(ray.dir, worldUp), 0.0f), 0.5f);

		skyCol = glm::mix(skyBase, skyTop, upAmount) * params.skyIntensity;
	}

	if (params.enableSun) {
		float sunAngle = glm::acos(glm::dot(ray.dir, params.sunDir));

		if (glm::degrees(sunAngle) < params.sunAngle) {
			skyCol = params.sunColor * params.sunIntensity;
		}
	}

	return skyCol;
}

//void PathTracer::sampleSun(PathRay& ray, std::vector<Tri>& tris, Params& params, bool& isShadow) {
//
//	PathRay sunRay;
//
//	sunRay.src = ray.hitPos + tris[ray.triIdx].normal * 0.001f;
//
//	static thread_local std::mt19937 rng(std::random_device{}());
//	static thread_local std::uniform_real_distribution<float> dist(0.0f, 1.0f);
//
//	glm::vec3 sunDir = params.sunDir;
//	sunDir = glm::normalize(sunDir);
//
//	float phi = 2.0f * PI * dist(rng);
//
//	float halfAngle = glm::radians(params.sunAngle * 0.5f);
//	float cosMax = std::cos(halfAngle);
//	float cosTheta = 1.0f - dist(rng) * (1.0f - cosMax);
//	float sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);
//
//	glm::vec3 worldUp = { 0.0f, 0.0f, 1.0f };
//	worldUp = glm::normalize(worldUp);
//
//	if (sunDir == worldUp) {
//		worldUp = { 0.0f, 0.5f, 1.0f };
//		worldUp = glm::normalize(worldUp);
//	}
//
//	glm::vec3 tangent = glm::normalize(glm::cross(sunDir, worldUp));
//	glm::vec3 bitangent = glm::normalize(glm::cross(sunDir, tangent));
//
//	sunRay.dir = glm::normalize(
//		sinTheta * std::cos(phi) * tangent +
//		sinTheta * std::sin(phi) * bitangent +
//		cosTheta * sunDir
//	);
//	sunRay.invDir = 1.0f / sunRay.dir;
//
//	float closestTSun = FLT_MAX;
//	sunRay.hit = false;
//	sunRay.triIdx = UINT32_MAX;
//
//	traverseFlatBVH(sunRay, closestTSun, tris, globalCompactBVH);
//
//	if (!sunRay.hit) {
//		ray.col += ray.throughput * params.skyIntensity * sky(sunRay, params) / params.sunIntensity;
//	}
//	else {
//		isShadow = true;
//	}
//
//	// REPLACE THE SKY LOGIC IN rayLogic(); WITH THIS WHEN USING SUN SAMPLING
//
//	//if (!ray.hit) {
//		//	ray.active = false;
//
//		//	if (params.environmentLight) {
//
//
//		//		ray.col += ray.throughput * params.environmentIntensity * sky(ray, params);
//		//		/*if (!isShadow) {
//		//			float sunAngle = glm::acos(glm::dot(ray.dir, params.sunDir) / (glm::length(ray.dir) * glm::length(params.sunDir)));
//		//			if (glm::degrees(sunAngle) > params.sunAngle) {
//		//				ray.col += ray.throughput * params.environmentIntensity * sky(ray, params);
//		//			}
//		//		}
//		//		else {
//		//			ray.col += ray.throughput * params.environmentIntensity * sky(ray, params);
//		//		}*/
//		//	}
//		//	/*else if (params.environmentLight && bounce == 0) {
//		//		ray.col += ray.throughput * params.environmentIntensity * sky(ray, params);
//		//	}*/
//
//		//	break;
//		//}
//}

std::vector<DebugRay> PathTracer::rayLogic(PathRay& ray, PathRayState& rayState, std::vector<Tri>& tris, Params& params, bool debug) {

	debugRays.clear();

	for (int bounce = 0; bounce <= params.maxBounces; bounce++) {

		thread_local std::mt19937 rng(std::random_device{}());
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);

		if (!rayState.active) {
			break;
		}

		float closestT = FLT_MAX;

		rayState.hit = false;
		rayState.triIdx = UINT32_MAX;

		traverseFlatBVH(ray, rayState, closestT, tris, globalCompactBVH);

		if (debug) {
			float drawLength = closestT;
			if (closestT == FLT_MAX) {
				drawLength = 1000.0f;
			}

			debugRays.push_back({ ray.src, ray.dir, rayState.throughput, drawLength });
		}

		if (rayState.isVolume && rayState.triIdx != UINT32_MAX) {
			float randomVal = std::max(dist(rng), 0.0001f);
			float scatterDist = -std::log(randomVal) / tris[rayState.triIdx].density;

			if (scatterDist < closestT) {

				rayState.hitPos = ray.src + ray.dir * scatterDist;
				ray.src = rayState.hitPos;

				glm::vec3 randDir;
				do {
					randDir = glm::vec3(dist(rng) * 2.0f - 1.0f, dist(rng) * 2.0f - 1.0f, dist(rng) * 2.0f - 1.0f);
				} while (glm::length(randDir) > 1.0f || glm::length(randDir) < 0.001f);

				ray.dir = glm::normalize(randDir);
				ray.invDir = 1.0f / ray.dir;

				rayState.throughput *= tris[rayState.triIdx].volumeCol;

				continue;
			}
		}

		if (rayState.triIdx != UINT32_MAX && rayState.active) {

			glm::vec3 interpolatedNormal = InterpolateNormal(rayState, tris);

			float cosTheta = glm::dot(ray.dir, interpolatedNormal);
			bool isInside = cosTheta > 0.0f;

			glm::vec3 orientedNormal = isInside ? -interpolatedNormal : interpolatedNormal;

			ray.src = rayState.hitPos + tris[rayState.triIdx].normal * 0.001f;

			float emissionVal = (tris[rayState.triIdx].emissionCol.x + tris[rayState.triIdx].emissionCol.y + tris[rayState.triIdx].emissionCol.z) / 3.0f;
			bool isEmissive = tris[rayState.triIdx].emissionIntensity > 0.0f && emissionVal > 0.0f;

			if (isEmissive) {
				rayState.col += rayState.throughput * tris[rayState.triIdx].emissionCol * tris[rayState.triIdx].emissionIntensity;
			}

			bool isVolumeMaterial = dist(rng) < tris[rayState.triIdx].volume;

			if (isVolumeMaterial) {
				ray.src = rayState.hitPos - orientedNormal * 0.001f;

				if (!isInside) {
					rayState.isVolume = true;
				}
				else {
					rayState.isVolume = false;
				}
			}
			else {

				bool isSpecular = false;

				isSpecular = specularLighting(ray, rayState, interpolatedNormal, tris);

				if (!isSpecular) {
					if (dist(rng) < tris[rayState.triIdx].refraction) {
						refractionLighting(ray, rayState, interpolatedNormal, tris);
					}
					else {
						rayState.throughput *= tris[rayState.triIdx].albedo;

						diffuseLighting(ray, rayState, interpolatedNormal, tris);
					}
				}
				else {
					rayState.throughput *= tris[rayState.triIdx].specularCol;
				}
			}
		}

		if (!rayState.hit && !rayState.isVolume) {
			rayState.active = false;

			rayState.col += rayState.throughput * sky(ray, params);

			break;
		}

		ray.invDir = 1.0f / ray.dir;
	}

	return debugRays;
}

void PathTracer::rayGeneration(std::vector<PathRay>& rays, std::vector<PathRayState>& rayStates, PTCam& myCam, Screen& screen, Params& params) {

#pragma omp parallel for collapse(2)
	for (int y = 0; y < screen.resY; y++) {
		for (int x = 0; x < screen.resX; x++) {
			thread_local std::mt19937 rng(std::random_device{}());
			std::uniform_real_distribution<float> unitDist(0.0f, 1.0f);

			float jitterX = unitDist(rng) - 0.5f;
			float jitterY = unitDist(rng) - 0.5f;

			float angle = unitDist(rng) * 2.0f * PI;
			float radius = myCam.aperture * sqrtf(unitDist(rng));
			glm::vec2 diskSample = { glm::cos(angle) * radius, glm::sin(angle) * radius };

			float srcOffsetX = (static_cast<float>(x) + 0.5f + jitterX * params.blur) / static_cast<float>(screen.resX) - 0.5f;
			float srcOffsetY = (static_cast<float>(y) + 0.5f + jitterY * params.blur) / static_cast<float>(screen.resY) - 0.5f;

			glm::vec3 src = myCam.camPos;

			src -= myCam.right * (srcOffsetX) * (myCam.sensorSize / 1000.0f);
			src += myCam.up * (srcOffsetY) * ((myCam.sensorSize / 1000.0f) / screen.ratio);

			glm::vec3 dir = glm::normalize(myCam.focalPoint - src);

			glm::vec3 focusPoint = myCam.camPos + (dir * (myCam.focusDist / glm::dot(dir, myCam.camNormal)));

			src -= myCam.right * (diskSample.x) * (myCam.sensorSize / 1000.0f);
			src += myCam.up * (diskSample.y) * ((myCam.sensorSize / 1000.0f) / screen.ratio);

			dir = glm::normalize(focusPoint - src);

			int index = y * screen.resX + x;

			rays[index].src = src;
			rays[index].dir = dir;
			rays[index].invDir = 1.0f / dir;
			rayStates[index].hitPos = src;
			rayStates[index].col = glm::vec3(0.0f);
			rayStates[index].throughput = glm::vec3(1.0f * myCam.ISO);
			rayStates[index].length = FLT_MAX;
			rayStates[index].triIdx = FLT_MAX;
			rayStates[index].hit = false;
			rayStates[index].active = true;
			rayStates[index].isRefraction = false;
			rayStates[index].isVolume = false;
		}
	}
}

void PathTracer::render(Data& data, PTCam& myCam, Screen& screen, Params& params, Texture2D& render) {

	if (params.shouldSample) {
		if (params.currentSample < params.maxSamples) {

			for (int rays = 0; rays < params.raysPerPixel; rays++) {
				rayGeneration(data.rays, data.rayStates, myCam, screen, params);

#pragma omp parallel for
				for (int i = 0; i < data.rays.size(); i++) {
					rayLogic(data.rays[i], data.rayStates[i], data.tris, params);
				}

				for (size_t i = 0; i < data.frameBuffer.size(); i++) {
					data.accumBuffer[i] += data.rayStates[i].col;
				}
			}

			params.currentSample++;
		}
	}
	else {

		params.currentSample = 1;

		for (size_t i = 0; i < data.frameBuffer.size(); i++) {

			data.accumBuffer[i] = { 0.0f, 0.0f, 0.0f };
		}

		for (int rays = 0; rays < params.raysPerPixel; rays++) {

			rayGeneration(data.rays, data.rayStates, myCam, screen, params);

#pragma omp parallel for
			for (int i = 0; i < data.rays.size(); i++) {
				rayLogic(data.rays[i], data.rayStates[i], data.tris, params);
			}

			for (size_t i = 0; i < data.frameBuffer.size(); i++) {
				data.accumBuffer[i] += data.rayStates[i].col;
			}
		}
	}

	drawScreen(screen, params, data, screen.resX, render);
}
