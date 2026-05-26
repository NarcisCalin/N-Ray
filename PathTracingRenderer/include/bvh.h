#pragma once
#include <glm/glm.hpp>
#include <tri.h>

struct BVH;
extern std::vector<BVH> globalBVH;

struct BVH {
	glm::vec3 min;
	glm::vec3 max;
	glm::vec3 splitPoint = { 0.0f, 0.0f, 0.0f };

	uint32_t children[2] = { UINT32_MAX, UINT32_MAX };

	uint32_t startIndex;
	uint32_t endIndex;
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

	BVH() = default;

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