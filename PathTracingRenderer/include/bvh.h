#pragma once
#include <glm/glm.hpp>
#include <tri.h>
#include <rtcore.h>

struct EmbreeBVH {

	std::vector<glm::vec3> vertices;
	std::vector<Tri>* tris = nullptr;
	RTCDevice device = nullptr;
	RTCScene scene = nullptr;

	EmbreeBVH() {
		device = rtcNewDevice(nullptr);
		scene = rtcNewScene(device);
	}

	~EmbreeBVH() {
		if (scene)  rtcReleaseScene(scene);
		if (device) rtcReleaseDevice(device);
	}

	EmbreeBVH(const EmbreeBVH&) = delete;
	EmbreeBVH& operator=(const EmbreeBVH&) = delete;

	EmbreeBVH(EmbreeBVH&& other) noexcept
		: vertices(std::move(other.vertices)),
		tris(other.tris),
		device(other.device),
		scene(other.scene) {

		other.device = nullptr;
		other.scene = nullptr;
		other.tris = nullptr;
	}

	EmbreeBVH& operator=(EmbreeBVH&& other) noexcept {
		if (this != &other) {
			if (scene)  rtcReleaseScene(scene);
			if (device) rtcReleaseDevice(device);
			vertices = std::move(other.vertices);
			tris = other.tris;
			device = other.device;
			scene = other.scene;
			other.device = nullptr;
			other.scene = nullptr;
			other.tris = nullptr;
		}
		return *this;
	}

	void build(std::vector<Tri>& inputTris) {
		tris = &inputTris;

		vertices.clear();
		vertices.reserve(inputTris.size() * 3);
		for (const Tri& t : inputTris) {
			vertices.push_back(t.a);
			vertices.push_back(t.b);
			vertices.push_back(t.c);
		}

		const size_t triCount = inputTris.size();
		const size_t vertexCount = vertices.size();

		RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

		glm::vec3* v = (glm::vec3*)rtcSetNewGeometryBuffer(
			geom,
			RTC_BUFFER_TYPE_VERTEX, 0,
			RTC_FORMAT_FLOAT3,
			sizeof(glm::vec3),
			vertexCount
		);

		for (size_t i = 0; i < vertexCount; i++)
			v[i] = vertices[i];

		uint32_t* idx = (uint32_t*)rtcSetNewGeometryBuffer(
			geom,
			RTC_BUFFER_TYPE_INDEX, 0,
			RTC_FORMAT_UINT3,
			3 * sizeof(uint32_t),
			triCount
		);

		for (size_t i = 0; i < triCount; i++) {
			idx[i * 3 + 0] = (uint32_t)(i * 3 + 0);
			idx[i * 3 + 1] = (uint32_t)(i * 3 + 1);
			idx[i * 3 + 2] = (uint32_t)(i * 3 + 2);
		}

		rtcCommitGeometry(geom);
		rtcAttachGeometry(scene, geom);
		rtcReleaseGeometry(geom);
		rtcCommitScene(scene);
	}
};

struct BVH;
struct CompactBVH;
extern std::vector<BVH> globalBVH;
extern std::vector<CompactBVH> globalCompactBVH;

struct BVH {
	glm::vec3 min = glm::vec3(0.0f);
	glm::vec3 max = glm::vec3(0.0f);
	glm::vec3 splitPoint = glm::vec3(0.0f);

	uint32_t children[2] = { UINT32_MAX, UINT32_MAX };

	uint32_t startIndex = 0;
	uint32_t endIndex = 0;
	uint32_t next = 0;

	BVH(uint32_t startIndex, uint32_t endIndex, std::vector<Tri>& tris, std::vector<BVH>& globalBVH) :
		startIndex(startIndex), endIndex(endIndex) {

		calculateAABB(tris);
		avgSplit(tris);

		uint32_t count = 0;
		if (endIndex >= startIndex && startIndex < tris.size()) {
			uint32_t clampedEnd = std::min<uint32_t>(endIndex, uint32_t(tris.size() - 1));
			count = clampedEnd - startIndex + 1;
		}

		if (count >= 3) {
			createChildren(tris, globalBVH);
			calculateNextNeighbor();
		}
	}

	BVH()
		: min(0.0f),
		max(0.0f),
		splitPoint(0.0f),
		startIndex(0),
		endIndex(0),
		next(0) {

		children[0] = UINT32_MAX;
		children[1] = UINT32_MAX;
	}

	void calculateAABB(std::vector<Tri>& tris) {
		min = glm::vec3(std::numeric_limits<float>::max());
		max = glm::vec3(std::numeric_limits<float>::lowest());

		if (startIndex > endIndex || startIndex >= tris.size()) return;

		uint32_t clampedEnd = std::min<uint32_t>(endIndex, uint32_t(tris.size() - 1));
		for (uint32_t i = startIndex; i <= clampedEnd; ++i) {
			min = glm::min(min, tris[i].min);
			max = glm::max(max, tris[i].max);
		}
	}

	void avgSplit(std::vector<Tri>& tris) {
		splitPoint = glm::vec3(0.0f);

		if (startIndex > endIndex || startIndex >= tris.size()) return;

		uint32_t clampedEnd = std::min<uint32_t>(endIndex, uint32_t(tris.size() - 1));
		uint32_t count = clampedEnd - startIndex + 1;
		if (count == 0) return;

		for (uint32_t i = startIndex; i <= clampedEnd; ++i) {
			splitPoint += tris[i].center;
		}

		splitPoint /= float(count);
	}

	void createChildren(std::vector<Tri>& tris, std::vector<BVH>& globalBVH) {

		glm::vec3 extent = max - min;
		int axis = 0;
		if (extent.y > extent.x) axis = 1;
		if (extent.z > extent[axis]) axis = 2;

		uint32_t aIdx = startIndex;

		for (uint32_t i = startIndex; i <= endIndex && i < tris.size(); ++i) {
			bool isChildA = tris[i].center[axis] < splitPoint[axis];

			if (isChildA) {
				if (i == aIdx) {
					aIdx++;
					continue;
				}

				std::swap(tris[i], tris[aIdx]);

				tris[i].id = i;
				tris[aIdx].id = aIdx;

				aIdx++;
			}
		}

		uint32_t leftCount = (aIdx > startIndex) ? (aIdx - startIndex) : 0;
		uint32_t rightCount = (endIndex >= aIdx && aIdx < tris.size()) ? (endIndex - aIdx + 1) : 0;

		if (leftCount == 0 || rightCount == 0) {
			return;
		}

		uint32_t childAIdx = uint32_t(globalBVH.size());
		globalBVH.emplace_back();
		globalBVH[childAIdx] = BVH(startIndex, aIdx - 1, tris, globalBVH);

		uint32_t childBIdx = uint32_t(globalBVH.size());
		globalBVH.emplace_back();
		globalBVH[childBIdx] = BVH(aIdx, endIndex, tris, globalBVH);

		children[0] = childAIdx;
		children[1] = childBIdx;
	}

	inline void calculateNextNeighbor() {

		next = 0;

		for (int i = 0; i < 2; ++i) {
			uint32_t idx = children[i];

			if (idx == UINT32_MAX) continue;

			BVH& child = globalBVH[idx];

			next += child.next;

			next++;
		}
	}
};

struct CompactBVH {
	glm::vec3 min;
	glm::vec3 max;

	union {
		uint32_t startIndex;
		uint32_t missLink;
	};

	uint32_t triCount;
};