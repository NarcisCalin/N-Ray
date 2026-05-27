#include "ui.h"

bool UI::sliderHelper(std::string label, std::string tooltip, glm::vec2 size, float& parameter, float minVal, float maxVal,
	bool isEnabled) {

	bool isSliderUsed = false;

	ImGuiID sliderId = ImGui::GetID(label.c_str());
	static std::unordered_map<ImGuiID, bool> hoverStates;
	static std::unordered_map<ImGuiID, float> defaultValues;

	if (!isEnabled) {
		ImGui::BeginDisabled();
	}

	if (defaultValues.find(sliderId) == defaultValues.end()) {
		defaultValues[sliderId] = parameter;
	}

	ImVec2 sliderSize;

	if (size.x > 0.0f && size.y > 0.0f) {
		sliderSize = ImVec2(size.x, size.y);
	}
	else if (size.x < 0.0f && size.y > 0.0f) {
		sliderSize = ImVec2(ImGui::GetContentRegionAvail().x, size.y);
	}
	else if (size.x > 0.0f && size.y < 0.0f) {
		sliderSize = ImVec2(size.x, ImGui::GetContentRegionAvail().y);
	}
	else {
		sliderSize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
	}

	ImGui::Text("%s", label.c_str());

	if (ImGui::SliderFloat(("##" + label).c_str(), &parameter, minVal, maxVal, "%.3f")) {
		isSliderUsed = true;
	}

	std::vector<Sound>* soundPool = nullptr;

	static bool hasBeenPressed = false;

	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {

		hasBeenPressed = true;

		
	}

	if (hasBeenPressed && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {

		

		hasBeenPressed = false;
	}

	static float prevValue = parameter;
	static bool wasPlaying = false;
	static ImVec2 lastMousePos = ImGui::GetMousePos();

	if (ImGui::IsItemActive()) {
		ImVec2 currentMousePos = ImGui::GetMousePos();
		float mouseDelta = abs(currentMousePos.x - lastMousePos.x) + abs(currentMousePos.y - lastMousePos.y);

		if (mouseDelta > 2.0f) {
			if (!wasPlaying || parameter != prevValue) {
				
			}
		}
		prevValue = parameter;
	}
	else {
		wasPlaying = false;
	}

	bool isHovered = ImGui::IsItemHovered();

	if (isHovered) {
		ImGui::SetTooltip("%s", tooltip.c_str());

		int randSoundNum = rand() % 3;

		if (!hoverStates[sliderId]) {
			
		}
	}

	hoverStates[sliderId] = isHovered;

	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
		parameter = defaultValues[sliderId];

		isSliderUsed = true;
	}

	if (!isEnabled) {
		ImGui::EndDisabled();
	}

	return isSliderUsed;
}
