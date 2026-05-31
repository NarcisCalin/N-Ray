#include "ui.h"

void UI::logic(Params& params, Data& data, PTCam& myCam) {

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

	if (sliderHelper("Sun Dir X", "Sets sun direction X", sliderSize, params.sunDir.x, 0.0f, 1.0f)) {
		params.shouldSample = false;
		params.sunDir = glm::normalize(params.sunDir);
	}

	if (sliderHelper("Sun Dir Y", "Sets sun direction Y", sliderSize, params.sunDir.y, 0.0f, 1.0f)) {
		params.shouldSample = false;
		params.sunDir = glm::normalize(params.sunDir);
	}

	if (sliderHelper("Sun Dir Z", "Sets sun direction Z", sliderSize, params.sunDir.z, 0.0f, 1.0f)) {
		params.shouldSample = false;
		params.sunDir = glm::normalize(params.sunDir);
	}

	if (sliderHelper("Sun Angle", "Sets sun size in degrees", sliderSize, params.sunAngle, 0.0f, 90.0f)) {
		params.shouldSample = false;
	}

	if (sliderHelper("Sun Intensity", "Sets sun intensity", sliderSize, params.sunIntensity, 0.0f, 100.0f)) {
		params.shouldSample = false;
	}

	if (sliderHelper("Sky Intensity", "Sets sky intensity", sliderSize, params.skyIntensity, 0.0f, 10.0f)) {
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

	if (buttonHelper("Enable Sun", "Enables sun", buttonSize, params.enableSun)) {
		params.shouldSample = false;
	}

	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Spacing();
	ImGui::Separator();

	if (buttonHelper("Enable Sky", "Enables sky", buttonSize, params.enableSky)) {
		params.shouldSample = false;
	}

	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Render Settings");

	ImGui::Separator();
	ImGui::Spacing();

	if (sliderHelper("Bounces Amount", "Amount of times a ray can bounce", sliderSize, params.maxBounces, 0, 50)) {
		params.shouldSample = false;
	}

	if (sliderHelper("Max Samples", "Max amount of samples to render", sliderSize, params.maxSamples, 1, 50000)) {
		params.shouldSample = false;
	}

	if (sliderHelper("Rays Per Pixel", "Amount of rays each pixel traces per sample", sliderSize, params.raysPerPixel, 1, 8)) {
		params.shouldSample = false;
	}

	if (sliderHelper("Resolution", "Sets image resolution size", sliderSize, params.res, 16, 1024)) {
		params.shouldSample = false;
	}

	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Camera Settings");

	ImGui::Separator();
	ImGui::Spacing();

	if (sliderHelper("Antialiasing Blur", "Sets how blurry the image renders", sliderSize, params.blur, 0.0f, 10.0f)) {
		params.shouldSample = false;
	}

	if (sliderHelper("Sensor Size", "Sets sensor size in mm.", sliderSize, myCam.sensorSize, 1.0f, 200.0f)) {
		params.shouldSample = false;
	}

	if (sliderHelper("Focal Length", "Sets focal length in mm.", sliderSize, myCam.focalLengthMM, 1.0f, 1000.0f)) {
		params.shouldSample = false;
	}

	if (sliderHelper("Aperture", "Controls camera aperture for DOF", sliderSize, myCam.aperture, 0.0f, 10.0f)) {
		params.shouldSample = false;
	}

	if (sliderHelper("Focus Distance", "How far from the camera is the focus point", sliderSize, myCam.focusDist, 0.0f, 100.0f)) {
		params.shouldSample = false;
	}

	if (sliderHelper("ISO", "ISO for camera exposure", sliderSize, myCam.ISO, 0.0f, 5.0f)) {
		params.shouldSample = false;
	}

	ImGui::Spacing();
	ImGui::Separator();

	buttonHelper("Pick DOF", "Lets you set a focus point by clicking the scene", buttonSize, myCam.clickDof);

	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Post Settings");

	sliderHelper("Exposure", "Controls image exposure after rendering", sliderSize, params.exposure, 0.0f, 5.0f, LogSlider);

	sliderHelper("Contras", "Controls image contrast", sliderSize, params.contrast, 0.0f, 2.0f, LogSlider);

	ImGui::Separator();
	ImGui::Spacing();

	ImGui::End();

	float ms = params.dt * 1000.0f;

	params.totalMs += ms;

	avgMsFrames[avgMsIdx] = ms;

	float avgMs = 0.0f;

	float totalMs = 0.0f;
	for (size_t i = 0; i < avgMsIdxAmount; i++) {
		totalMs += avgMsFrames[i];
	}

	avgMs = totalMs / float(avgMsIdxAmount);

	avgMsIdx++;

	if (avgMsIdx >= avgMsIdxAmount) {
		avgMsIdx = 0;
	}

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

bool UI::sliderHelper(std::string label, std::string tooltip, glm::vec2 size, float& parameter, float minVal, float maxVal, int logarithmic,
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

	if (ImGui::SliderFloat(("##" + label).c_str(), &parameter, minVal, maxVal, "%.3f", ImGuiSliderFlags_Logarithmic)) {
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

	bool pushedColors = false;

	if (parameter) {
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));
		pushedColors = true;
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

	if (pushedColors) {
		ImGui::PopStyleColor(3);
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
