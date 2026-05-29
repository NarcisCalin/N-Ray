#include "ui.h"

void UI::logic(Params& params, Data& data) {

	glm::vec2 sliderSize = { 200.0f, 30.0f };
	glm::vec2 buttonSize = { 150.0f, 30.0f };

	ImGui::SetNextWindowSize(ImVec2(200.0f, params.screenSize.y), ImGuiCond_Once);
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Once);
	ImGui::Begin("Settings", nullptr);

	ImGui::Spacing();
	ImGui::Separator();

	ImGui::Text("Sky Settings");

	ImGui::Separator();
	ImGui::Spacing();

	if (UI::sliderHelper("Sun Dir X", "Sets sun direction X", sliderSize, params.sunDir.x, 0.0f, 1.0f)) {
		params.shouldSample = false;
	}
	if (UI::sliderHelper("Sun Dir Y", "Sets sun direction Y", sliderSize, params.sunDir.y, 0.0f, 1.0f)) {
		params.shouldSample = false;
	}
	if (UI::sliderHelper("Sun Dir Z", "Sets sun direction Z", sliderSize, params.sunDir.z, 0.0f, 1.0f)) {
		params.shouldSample = false;
	}
	if (UI::sliderHelper("Sun Angle", "Sets sun size in degrees", sliderSize, params.sunAngle, 0.0f, 90.0f)) {
		params.shouldSample = false;
	}
	if (UI::sliderHelper("Sun Intensity", "Sets sun intensity", sliderSize, params.sunIntensity, 0.0f, 100.0f)) {
		params.shouldSample = false;
	}

	ImGui::Spacing();
	ImGui::Separator();

	if (ImGui::ColorEdit3("Sun Color", (float*)&params.sunColor, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoInputs)) {
		params.shouldSample = false;
	}

	if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
		params.sunColor = glm::vec3(1.0f);
		params.shouldSample = false;
	}

	ImGui::Spacing();
	ImGui::Separator();

	if (UI::buttonHelper("Enable Sun", "Enables sun", buttonSize, params.enableSun)) {
		params.shouldSample = false;
	}

	ImGui::Spacing();
	ImGui::Separator();

	ImGui::Text("Render Settings");

	ImGui::Separator();
	ImGui::Spacing();

	if (UI::sliderHelper("Bounces Amount", "Amount of times a ray can bounce", sliderSize, params.maxBounces, 0, 50)) {
		params.shouldSample = false;
	}

	ImGui::End();

	float ms = params.dt * 1000.0f;

	params.totalMs += ms;

	float avgMs = params.totalMs / float(params.totalFrames);

	ImGui::SetNextWindowSize(ImVec2(250.0f, 400.0f), ImGuiCond_Once);
	ImGui::SetNextWindowPos(ImVec2(params.screenSize.x - 250.0f, 0.0f), ImGuiCond_Once);
	ImGui::Begin("Stats", nullptr);

	ImGui::Text("FPS: %.d", GetFPS());

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Frame time: %.2f", ms);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Avg. Frame time: %.2f ms", avgMs);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Triangles: %d", (int)data.tris.size());

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Current Sample: %d", params.currentSample);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Max Samples: %d", params.maxSamples);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Bounces: %d", params.maxBounces);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Total time (ms): %.2f ms", params.totalMs);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Total time (sec): %.2f sec", params.totalMs / 1000.0f);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::End();

}

bool UI::sliderHelper(std::string label, std::string tooltip, glm::vec2 size, float& parameter, float minVal, float maxVal,
	bool isEnabled) {

	bool isSliderUsed = false;

	ImGuiID sliderId = ImGui::GetID(label.c_str());
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

	static float prevValue = parameter;
	static ImVec2 lastMousePos = ImGui::GetMousePos();

	if (ImGui::IsItemActive()) {
		ImVec2 currentMousePos = ImGui::GetMousePos();
		float mouseDelta = abs(currentMousePos.x - lastMousePos.x) + abs(currentMousePos.y - lastMousePos.y);

		prevValue = parameter;
	}

	bool isHovered = ImGui::IsItemHovered();

	if (isHovered) {
		ImGui::SetTooltip("%s", tooltip.c_str());
	}

	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
		parameter = defaultValues[sliderId];

		isSliderUsed = true;
	}

	if (!isEnabled) {
		ImGui::EndDisabled();
	}

	return isSliderUsed;
}

bool UI::sliderHelper(std::string label, std::string tooltip, glm::vec2 size, int& parameter, int minVal, int maxVal,
	bool isEnabled) {

	bool isSliderUsed = false;

	ImGuiID sliderId = ImGui::GetID(label.c_str());
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

	if (ImGui::SliderInt(("##" + label).c_str(), &parameter, minVal, maxVal)) {
		isSliderUsed = true;
	}

	static float prevValue = parameter;
	static ImVec2 lastMousePos = ImGui::GetMousePos();

	if (ImGui::IsItemActive()) {
		ImVec2 currentMousePos = ImGui::GetMousePos();
		float mouseDelta = abs(currentMousePos.x - lastMousePos.x) + abs(currentMousePos.y - lastMousePos.y);

		prevValue = parameter;
	}

	bool isHovered = ImGui::IsItemHovered();

	if (isHovered) {
		ImGui::SetTooltip("%s", tooltip.c_str());
	}

	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
		parameter = defaultValues[sliderId];

		isSliderUsed = true;
	}

	if (!isEnabled) {
		ImGui::EndDisabled();
	}

	return isSliderUsed;
}

bool UI::buttonHelper(std::string label, std::string tooltip, glm::vec2 size, bool& parameter, bool canDeactivateSelf, bool isEnabled) {

	ImGuiID buttonId = ImGui::GetID(label.c_str());
	static std::unordered_map<ImGuiID, bool> hoverStates;

	if (!isEnabled) {
		ImGui::BeginDisabled();
	}

	bool hasBeenPressed = false;
	ImVec2 buttonSize;

	if (size.x > 0.0f && size.y > 0.0f) {
		buttonSize = ImVec2(size.x, size.y);
	}
	else if (size.x < 0.0f && size.y > 0.0f) {
		buttonSize = ImVec2(ImGui::GetContentRegionAvail().x, size.y);
	}
	else if (size.x > 0.0f && size.y < 0.0f) {
		buttonSize = ImVec2(size.x, ImGui::GetContentRegionAvail().y);
	}
	else {
		buttonSize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
	}

	if (ImGui::Button(label.c_str(), buttonSize)) {

		if (canDeactivateSelf) {
			parameter = !parameter;
		}
		else if (!parameter) {
			parameter = true;
		}
		hasBeenPressed = true;
	}

	bool isHovered = ImGui::IsItemHovered();

	if (isHovered) {
		ImGui::SetTooltip("%s", tooltip.c_str());
	}

	if (!isEnabled) {
		ImGui::EndDisabled();
	}

	return hasBeenPressed;
}
