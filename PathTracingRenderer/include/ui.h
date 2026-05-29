#pragma once
#include <imgui.h>
#include <rlImGui.h>
#include <string>
#include <unordered_map>
#include <glm.hpp>
#include <globalParams.h>

struct UI {

	void logic(Params& params, Data& data);

	static bool sliderHelper(std::string label, std::string tooltip, glm::vec2 size, float& parameter, float minVal, float maxVal,
		bool isEnabled = true);

	// Int Overload
	static bool sliderHelper(std::string label, std::string tooltip, glm::vec2 size, int& parameter, int minVal, int maxVal,
		bool isEnabled = true);

	static bool buttonHelper(std::string label, std::string tooltip, glm::vec2 size, bool& parameter, bool canDeactivateSelf = true,
		bool isEnabled = true);
};