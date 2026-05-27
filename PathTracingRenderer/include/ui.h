#pragma once
#include <imgui.h>
#include <rlImGui.h>
#include <string>
#include <unordered_map>
#include <glm.hpp>

struct UI {

	void logic() {
	}

	static bool sliderHelper(std::string label, std::string tooltip, glm::vec2 size, float& parameter, float minVal, float maxVal,
		bool isEnabled = true);
};