#include "renderer.h"
#include <random>

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

bool PathTracer::rayAABB(const PathRay& ray, glm::vec3& boxMin, glm::vec3& boxMax) {
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

	return tmax >= std::max(tmin, 0.0f);
}

void PathTracer::diffuseLighting(PathRay& ray, glm::vec3& normal, std::vector<Tri>& tris) {

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

	if (glm::dot(worldDir, tris[ray.triIdx].normal) < 0.0f) {
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

bool PathTracer::specularLighting(PathRay& ray, glm::vec3& normal, std::vector<Tri>& tris) {
	static thread_local std::mt19937 rng(std::random_device{}());
	static thread_local std::uniform_real_distribution<float> dist(0.0f, 1.0f);

	float r1 = dist(rng);
	float r2 = dist(rng);
	glm::vec3 microfacetNormal = sampleGGX(normal, tris[ray.triIdx].roughness, r1, r2);

	float r0Dielectric = ((airIOR - tris[ray.triIdx].IOR) / (airIOR + tris[ray.triIdx].IOR));
	r0Dielectric = r0Dielectric * r0Dielectric;

	float r0 = glm::mix(r0Dielectric, 1.0f, tris[ray.triIdx].metalness);

	float cosThetaI = glm::clamp(-glm::dot(ray.dir, microfacetNormal), 0.0f, 1.0f);
	float rTheta = r0 + (1.0f - r0) * std::pow(1.0f - cosThetaI, 5.0f);

	if (dist(rng) > rTheta) {
		return false;
	}

	glm::vec3 reflectedDir = glm::reflect(ray.dir, microfacetNormal);

	if (glm::dot(reflectedDir, normal) <= 0.0f ||
		glm::dot(reflectedDir, tris[ray.triIdx].normal) <= 0.0f) {
		return false;
	}

	ray.dir = reflectedDir;
	return true;
}

void PathTracer::refractionLighting(PathRay& ray, glm::vec3 normal, std::vector<Tri>& tris) {
	float n1 = airIOR;
	float n2 = tris[ray.triIdx].IOR;
	float cosi = glm::dot(ray.dir, normal);

	if (cosi > 0.0f) {
		std::swap(n1, n2);
		normal = -normal;
		cosi = -cosi;
		ray.isRefraction = false;
	}
	else {
		cosi = -cosi;
		ray.src = ray.hitPos - normal * 0.001f;
		ray.isRefraction = true;
	}

	float eta = n1 / n2;
	float k = 1.0f - eta * eta * (1.0f - cosi * cosi);

	if (k < 0.0f) {
		ray.dir = glm::reflect(ray.dir, normal);
		return;
	}

	ray.dir = glm::normalize(glm::refract(ray.dir, normal, eta));
}

void PathTracer::traverseFlatBVH(PathRay& ray, float& closestT, std::vector<Tri>& tris) {

	uint32_t idx = 0;
	uint32_t nodeCount = static_cast<uint32_t>(globalBVH.size());

	if (nodeCount == 0) return;

	while (idx < nodeCount) {
		BVH& node = globalBVH[idx];

		if (!rayAABB(ray, node.min, node.max)) {
			idx += node.next + 1;
			continue;
		}

		if (node.children[0] == UINT32_MAX && node.children[1] == UINT32_MAX) {

			if (node.startIndex <= node.endIndex && !tris.empty() && node.startIndex < tris.size()) {

				for (uint32_t i = node.startIndex; i <= node.endIndex; ++i) {

					float t;
					Tri& tri = tris[i];

					if (RayIntersectsTriangle(ray, tri, t)) {
						if (t < closestT) {
							closestT = t;
							ray.hit = true;
							ray.hitPos = ray.src + ray.dir * t;
							ray.triIdx = tri.idx;
						}
					}
				}
			}
			++idx;
			continue;
		}

		++idx;
	}
}

void PathTracer::directLight(PathRay& ray, glm::vec3 normal, std::vector<Tri>& tris, Params& params) {

	static thread_local std::mt19937 rng(std::random_device{}());
	thread_local std::uniform_int_distribution<size_t> idx(0, params.emissiveAmount - 1);
	thread_local std::uniform_real_distribution<float> uv(0.0f, 1.0f);

	size_t emIdx = idx(rng);

	float u = uv(rng);
	float v = uv(rng);

	if (u + v > 1.0f) {
		u = 1.0f - u;
		v = 1.0f - v;
	}

	glm::vec3 sampleP = u * tris[emIdx].a + v * tris[emIdx].b + (1.0f - u - v) * tris[emIdx].c;
	glm::vec3 sampleN = tris[emIdx].normal;

	glm::vec3 lVec = sampleP - ray.hitPos;
	float dSq = glm::dot(lVec, lVec);
	float dist = sqrt(dSq);
	glm::vec3 dlDir = lVec / dist;

	PathRay dlRay({ ray.hitPos + tris[ray.triIdx].normal * 0.001f, dlDir });

	float closestT = FLT_MAX;
	traverseFlatBVH(dlRay, closestT, tris);

	if (closestT < dist - 0.001f) {
		return;
	}

	glm::vec3 ab = tris[emIdx].a - tris[emIdx].b;
	glm::vec3 ac = tris[emIdx].a - tris[emIdx].c;

	float area = 0.5f * glm::length(glm::cross(ab, ac));

	float pdf = 1.0f / (float(params.emissiveAmount) * area);

	glm::vec3 dlDirT = glm::normalize(sampleP - ray.hitPos);
	glm::vec3 diff = sampleP - ray.hitPos;
	float distSq = glm::dot(diff, diff);

	float cosSurface = glm::max(glm::dot(normal, dlDirT), 0.0f);
	float cosLight = glm::max(glm::dot(sampleN, -dlDirT), 0.0f);

	float G = (cosSurface * cosLight) / distSq;

	glm::vec3 emission = tris[emIdx].emissionCol * tris[emIdx].emissionIntensity;

	ray.col += ray.throughput * emission * G / (pdf * PI);
}

glm::vec3 sky(PathRay& ray) {
	glm::vec3 worldUp = { 0.0f, 0.0f, 1.0f };

	glm::vec3 skyTop = { 0.263f, 0.553f, 0.769f };
	glm::vec3 skyBase = { 0.89f, 0.824f, 0.698f };

	float upAmount = glm::pow(glm::max(glm::dot(ray.dir, worldUp), 0.0f), 0.5f);

	glm::vec3 skyCol = glm::mix(skyBase, skyTop, upAmount);

	return skyCol;
}

void PathTracer::rayLogic(PathRay& ray, std::vector<Tri>& tris, Params& params) {
	for (int bounce = 0; bounce <= params.maxBounces; bounce++) {

		if (!ray.active) {
			break;
		}

		float closestT = FLT_MAX;
		ray.hit = false;
		ray.triIdx = UINT32_MAX;

		traverseFlatBVH(ray, closestT, tris);

		if (!ray.hit) {
			ray.active = false;

			if (params.environmentLight) {
				ray.col += ray.throughput * params.environmentIntensity * sky(ray);
			}

			if (params.sunLight) {
				thread_local std::mt19937 rng(std::random_device{}());
				thread_local std::uniform_real_distribution<float> sunRand(-params.sunAngle, params.sunAngle);

				float sunRayDiff = glm::dot(ray.dir, params.sunDir);
				if (sunRayDiff > params.sunCosAngle) {
					ray.col += ray.throughput * params.sunColor * params.sunIntensity;
				}
			}

			break;
		}

		if (ray.triIdx != UINT32_MAX && ray.active) {

			glm::vec3 interpolatedNormal = InterpolateNormal(ray, tris);
			ray.src = ray.hitPos + tris[ray.triIdx].normal * 0.001f;

			float emissionVal = (tris[ray.triIdx].emissionCol.x + tris[ray.triIdx].emissionCol.y + tris[ray.triIdx].emissionCol.z) / 3.0f;
			bool isEmissive = tris[ray.triIdx].emissionIntensity > 0.0f && emissionVal > 0.0f;

			if (isEmissive) {
				ray.col += ray.throughput * tris[ray.triIdx].emissionCol * tris[ray.triIdx].emissionIntensity;
			}

			bool isSpecular = specularLighting(ray, interpolatedNormal, tris);

			if (!isSpecular) {
				if (tris[ray.triIdx].refraction > 0.0f) {
					refractionLighting(ray, interpolatedNormal, tris);
				}
				else {
					ray.throughput *= tris[ray.triIdx].col;
					diffuseLighting(ray, interpolatedNormal, tris);
				}
			}
			else {
				ray.throughput *= tris[ray.triIdx].specularCol;
			}
		}
		ray.invDir = 1.0f / ray.dir;
	}
}

void PathTracer::rayGeneration(std::vector<PathRay>& rays, PTCam& myCam, Screen& screen, Params& params) {

	rays.resize(screen.resX * screen.resY);

#pragma omp parallel for collapse(2)
	for (int y = 0; y < screen.resY; y++) {
		for (int x = 0; x < screen.resX; x++) {

			thread_local std::mt19937 rng(std::random_device{}());
			thread_local std::uniform_real_distribution<float> dist(-params.blur, params.blur);

			float offsetX = ((static_cast<float>(x) + 0.5f + dist(rng)) / static_cast<float>(screen.resX) - 0.5f);
			float offsetY = ((static_cast<float>(y) + 0.5f + dist(rng)) / static_cast<float>(screen.resY) - 0.5f);

			glm::vec3 dir = myCam.camNormal;
			dir += myCam.right * offsetX * myCam.verticalScale * screen.ratio;
			dir -= myCam.up * offsetY * myCam.verticalScale;
			dir = glm::normalize(dir);

			int index = y * screen.resX + x;

			rays[index].src = myCam.camPos;
			rays[index].dir = dir;
			rays[index].invDir = 1.0f / rays[index].dir;

			rays[index].active = true;
			rays[index].col = glm::vec3(0.0f);
			rays[index].throughput = glm::vec3(1.0f);
		}
	}
}

void PathTracer::render(Data& data, PTCam& myCam, Screen& screen, Params& params, Texture2D& render) {

	if (params.shouldSample) {
		if (params.currentSample < params.maxSamples) {

			rayGeneration(data.rays, myCam, screen, params);

#pragma omp parallel for
			for (int i = 0; i < data.rays.size(); i++) {
				rayLogic(data.rays[i], data.tris, params);
			}

			for (size_t i = 0; i < data.frameBuffer.size(); i++) {
				PathRay& r = data.rays[i];

				data.accumBuffer[i] += r.col;
			}

			params.currentSample++;
		}
	}
	else {

		params.currentSample = 0;

		for (size_t i = 0; i < data.frameBuffer.size(); i++) {

			data.accumBuffer[i] = { 0.0f, 0.0f, 0.0f };
		}

		rayGeneration(data.rays, myCam, screen, params);

#pragma omp parallel for
		for (int i = 0; i < data.rays.size(); i++) {
			rayLogic(data.rays[i], data.tris, params);
		}

		for (size_t i = 0; i < data.frameBuffer.size(); i++) {
			PathRay& r = data.rays[i];

			data.accumBuffer[i] += r.col;
		}
	}

	drawScreen(screen, params, data, screen.resX, render);
}

std::vector<DebugRay> PathTracer::rayLogicDebug(PathRay ray, std::vector<Tri>& tris, Params& params) {

	std::vector<DebugRay> debugRays;

	for (int bounce = 0; bounce <= params.maxBounces; bounce++) {

		if (!ray.active) {
			break;
		}

		float closestT = FLT_MAX;
		ray.hit = false;
		ray.triIdx = UINT32_MAX;

		traverseFlatBVH(ray, closestT, tris);

		float drawLength = closestT;
		if (closestT == FLT_MAX) {
			drawLength = 1000.0f;
		}

		debugRays.push_back({ ray.src, ray.dir, ray.throughput, drawLength });

		if (!ray.hit) {
			ray.active = false;

			if (params.environmentLight) {
				ray.col += ray.throughput * params.environmentIntensity;
			}

			if (params.sunLight) {
				thread_local std::mt19937 rng(std::random_device{}());
				thread_local std::uniform_real_distribution<float> sunRand(-params.sunAngle, params.sunAngle);

				float sunRayDiff = glm::dot(ray.dir, params.sunDir);
				if (sunRayDiff > params.sunCosAngle) {
					ray.col += ray.throughput * params.sunColor * params.sunIntensity;
				}
			}

			break;
		}

		if (ray.triIdx != UINT32_MAX && ray.active) {

			glm::vec3 interpolatedNormal = InterpolateNormal(ray, tris);

			ray.src = ray.hitPos + tris[ray.triIdx].normal * 0.001f;

			float emissionVal = (tris[ray.triIdx].emissionCol.x + tris[ray.triIdx].emissionCol.y + tris[ray.triIdx].emissionCol.z) / 3.0f;
			bool isEmissive = tris[ray.triIdx].emissionIntensity > 0.0f && emissionVal > 0.0f;

			if (isEmissive) {
				ray.col += ray.throughput * tris[ray.triIdx].emissionCol * tris[ray.triIdx].emissionIntensity;
			}

			bool isSpecular = specularLighting(ray, interpolatedNormal, tris);

			if (!isSpecular) {
				if (tris[ray.triIdx].refraction > 0.0f) {
					refractionLighting(ray, interpolatedNormal, tris);
				}
				else {
					ray.throughput *= tris[ray.triIdx].col;
					diffuseLighting(ray, interpolatedNormal, tris);
				}
			}
			else {
				ray.throughput *= tris[ray.triIdx].specularCol;
			}
		}
	}

	return debugRays;
}
