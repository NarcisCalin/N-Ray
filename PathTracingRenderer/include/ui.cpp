#include "ui.h"

void UI::logic(Params& params, Data& data, PTCam& myCam, PathTracer& pt) {

	glm::vec2 sliderSize = { 200.0f, 30.0f };
	glm::vec2 buttonSize = { 170.0f, 30.0f };

	ImGui::SetNextWindowSize(ImVec2(200.0f, float(params.screenSize.y)), ImGuiCond_Once);
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Once);
	ImGui::Begin("Settings", nullptr);

	if (ImGui::CollapsingHeader("Render Settings")) {
		ImGui::Separator();

		if (buttonHelper("Render", "Renders scene with path tracing", buttonSize, params.render)) {
			params.shouldSample = false;
		}

		if (sliderHelper("Bounces Amount", "Amount of times a ray can bounce", sliderSize, params.maxBounces, 0, 50, params.rmPBR)) {
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
	}
	if (ImGui::CollapsingHeader("Engine Settings")) {


		if (buttonHelper("Path Tracer", "Enables standard path tracing", buttonSize, params.pathTracer)) {
			params.shouldSample = false;
		}

		if (buttonHelper("Ray Marcher", "Enables ray marching rendering", buttonSize, params.rayMarcher)) {
			params.shouldSample = false;
		}

		if (sliderHelper("RM Max Steps", "Maximum steps for ray marching", sliderSize, params.rmMaxSteps, 1, 1024, params.rayMarcher)) {
			params.shouldSample = false;
		}

		if (sliderHelper("RM Min Dist", "Minimum ray marching step length for collision detection", sliderSize, params.rmNearPlane, 0.0001f, 1.0f, LogSlider, params.rayMarcher)) {
			params.shouldSample = false;
		}

		if (sliderHelper("RM Max Dist", "If a ray step is larger than this, it stops marching", sliderSize, params.rmFarPlane, 1.0f, 1000.0f, LogSlider, params.rayMarcher)) {
			params.shouldSample = false;
		}

		if (buttonHelper("Ray Marcher PBR", "Enables PBR rendering for ray marching", buttonSize, params.rmPBR, true, params.rayMarcher)) {
			params.shouldSample = false;

			if (!params.rmPBR) {
				params.prevMaxBounces = params.maxBounces;
				params.maxBounces = 0;
			}
			else {
				params.maxBounces = params.prevMaxBounces;
			}
		}

		ImGui::Separator();
	}

	if (ImGui::CollapsingHeader("Sky Settings")) {
		ImGui::Separator();

		if (buttonHelper("Enable Sun", "Enables sun", buttonSize, params.enableSun)) {
			params.shouldSample = false;
		}

		if (buttonHelper("Enable Sky", "Enables sky (hdri must be disabled)", buttonSize, params.enableSky, true, !params.enableHDRI)) {
			params.shouldSample = false;
		}

		if (buttonHelper("Enable HDRI Map", "Enables HDRI map for lighting", buttonSize, params.enableHDRI)) {
			params.shouldSample = false;
		}

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

		if (sliderHelper("Sky Intensity", "Sets intensity for the sky / HDRI dome", sliderSize, params.skyIntensity, 0.0f, 10.0f, LogSlider)) {
			params.shouldSample = false;
		}

		if (sliderHelper("HDRI Rotation", "Rotation of the HDRI in degrees", sliderSize, params.hdriRotation, 0.0f, 360.0f)) {
			params.shouldSample = false;
		}

		if (sliderHelper("HDRI Threshold", "Rotation of the HDRI in degrees", sliderSize, params.hdriThreshold, 0.0f, 1.0f)) {
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

		if (ImGui::ColorEdit3("Sky Top Color", (float*)&params.skyTop, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoInputs)) {
			params.shouldSample = false;
		}

		if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
			params.skyTop = params.skyTopOriginal;
			params.shouldSample = false;
		}

		if (ImGui::ColorEdit3("Sky Base Color", (float*)&params.skyBase, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoInputs)) {
			params.shouldSample = false;
		}

		if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
			params.skyBase = params.skyBaseOriginal;
			params.shouldSample = false;
		}

		ImGui::Separator();
	}

	if (ImGui::CollapsingHeader("Camera Settings")) {
		ImGui::Separator();

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

		if (sliderHelper("Focus Distance", "How far from the camera is the focus point", sliderSize, myCam.focusDist, 0.0f, 100.0f, true, LogSlider)) {
			params.shouldSample = false;
		}

		if (sliderHelper("ISO", "ISO for camera exposure", sliderSize, myCam.ISO, 0.0f, 5.0f)) {
			params.shouldSample = false;
		}

		if (sliderHelper("Navigation Speed", "How fast the camera moves with WASD", sliderSize, myCam.camSpeed, 1.0f, 50.0f)) {
			params.shouldSample = false;
		}

		ImGui::Spacing();

		buttonHelper("Pick DOF", "Lets you set a focus point by clicking the scene", buttonSize, myCam.clickDof);

		ImGui::Separator();
	}

	if (ImGui::CollapsingHeader("Image Settings")) {
		ImGui::Separator();

		sliderHelper("Exposure", "Controls image exposure after rendering", sliderSize, params.exposure, 0.0f, 5.0f, LogSlider);

		sliderHelper("Contrast", "Controls image contrast", sliderSize, params.contrast, 0.0f, 2.0f, LogSlider);

		ImGui::Separator();
	}

	if (ImGui::CollapsingHeader("Ray Settings")) {
		ImGui::Separator();

		if (buttonHelper("Enable Caustics", "Enables caustics rendering", buttonSize, params.enableCaustics)) {
			params.shouldSample = false;
		}

		if (buttonHelper("Biased Caustics (Test)", "Enables biased caustics algorithm (It is an experiment and not truly physically based)", buttonSize, params.enableBiasCaustics, true, params.enableCaustics)) {
			params.shouldSample = false;
		}

		if (sliderHelper("Biased Caustics G", "This clamps the strength of the caustics when a refractive surface touches a diffuse surface", sliderSize, params.biasCausticsContactClamp, 0.1f, 5.0f)) {
			params.shouldSample = false;
		}

		sliderHelper("Caustics Intensity", "Caustics intensity", sliderSize, params.causticsIntensity, 0.0f, 1.0f);

		sliderHelper("Main Buffer Intensity", "Intensity of the base buffer (for example, caustics are not part of the main buffer", sliderSize, params.mainBufferIntensity, 0.0f, 1.0f);
	
		ImGui::Separator();
	}

	if (ImGui::CollapsingHeader("Scene Settings")) {
		ImGui::Separator();

		if (ImGui::CollapsingHeader("Path Tracer Material")) {
			buttonHelper("Enable Selection", "Allows selecting models", buttonSize, params.enableSelection);

			glm::vec3 newAlbedo = { 0.0f, 0.0f, 0.0f };
			glm::vec3 newSpecCol = { 0.0f, 0.0f, 0.0f };
			glm::vec3 newEmissionCol = { 0.0f, 0.0f, 0.0f };
			glm::vec3 newAbsorptionCol = { 0.0f, 0.0f, 0.0f };
			glm::vec3 newVolumeCol = { 0.0f, 0.0f, 0.0f };

			float newIOR = 0.0f;
			float newRoughness = 0.0f;
			float newEmissionIntensity = 0.0f;
			float newRefraction = 0.0f;
			float newAbsorption = 0.0f;
			float newVolume = 0.0f;
			float newDensity = 0.0f;
			float newMetalness = 0.0f;

			uint32_t selectedAmount = 0;

			for (size_t i = 0; i < data.models.size(); i++) {
				if (data.models[i].selected) {
					newAlbedo += data.models[i].albedo;
					newSpecCol += data.models[i].specularCol;
					newEmissionCol += data.models[i].emissionCol;
					newAbsorptionCol += data.models[i].absorptionCol;
					newVolumeCol += data.models[i].volumeCol;

					newIOR += data.models[i].IOR;
					newRoughness += data.models[i].roughness;
					newEmissionIntensity += data.models[i].emissionIntensity;
					newRefraction += data.models[i].refraction;
					newAbsorption += data.models[i].absorption;
					newVolume += data.models[i].volume;
					newDensity += data.models[i].density;
					newMetalness += data.models[i].metalness;

					selectedAmount++;
				}
			}

			if (selectedAmount > 0) {
				float inv = 1.0f / float(selectedAmount);

				newAlbedo *= inv;
				newSpecCol *= inv;
				newEmissionCol *= inv;
				newAbsorptionCol *= inv;
				newVolumeCol *= inv;

				newIOR *= inv;
				newRoughness *= inv;
				newEmissionIntensity *= inv;
				newRefraction *= inv;
				newAbsorption *= inv;
				newVolume *= inv;
				newDensity *= inv;
				newMetalness *= inv;
			}

			if (ImGui::ColorEdit3("Albedo Color", (float*)&newAlbedo, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoInputs)) {
				params.shouldSample = false;
			}

			if (ImGui::ColorEdit3("Specular Color", (float*)&newSpecCol, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoInputs)) {
				params.shouldSample = false;
			}

			if (ImGui::ColorEdit3("Emission Color", (float*)&newEmissionCol, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoInputs)) {
				params.shouldSample = false;
			}

			if (ImGui::ColorEdit3("Absorption Color", (float*)&newAbsorptionCol, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoInputs)) {
				params.shouldSample = false;
			}

			if (ImGui::ColorEdit3("Volume Color", (float*)&newVolumeCol, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoInputs)) {
				params.shouldSample = false;
			}

			if (sliderHelper("IOR", "Index of Refraction of selected models", sliderSize, newIOR, 0.0f, 200.0f, LogSlider)) {
				params.shouldSample = false;
			}

			if (sliderHelper("Roughness", "Roughness of selected models", sliderSize, newRoughness, 0.0f, 1.0f)) {
				params.shouldSample = false;
			}

			if (sliderHelper("Emission Intensity", "Emission intensity of selected models", sliderSize, newEmissionIntensity, 0.0f, 100.0f, LogSlider)) {
				params.shouldSample = false;
			}

			if (sliderHelper("Refraction", "Refraction of selected models", sliderSize, newRefraction, 0.0f, 1.0f)) {
				params.shouldSample = false;
			}

			if (sliderHelper("Absorption", "How much light a material absorbs when refracted", sliderSize, newAbsorption, 0.0f, 10.0f)) {
				params.shouldSample = false;
			}

			if (sliderHelper("Volume", "Volume scattering of selected models", sliderSize, newVolume, 0.0f, 1.0f)) {
				params.shouldSample = false;
			}

			if (sliderHelper("Density", "Volume density of selected models", sliderSize, newDensity, 0.0f, 100.0f)) {
				params.shouldSample = false;
			}

			if (sliderHelper("Metalness", "Metalness of selected models", sliderSize, newMetalness, 0.0f, 1.0f)) {
				params.shouldSample = false;
			}

			if (selectedAmount > 0) {
				for (size_t i = 0; i < data.models.size(); i++) {
					if (data.models[i].selected) {
						data.models[i].albedo = newAlbedo;
						data.models[i].specularCol = newSpecCol;
						data.models[i].emissionCol = newEmissionCol;
						data.models[i].absorptionCol = newAbsorptionCol;
						data.models[i].volumeCol = newVolumeCol;

						data.models[i].IOR = newIOR;
						data.models[i].roughness = newRoughness;
						data.models[i].emissionIntensity = newEmissionIntensity;
						data.models[i].refraction = newRefraction;
						data.models[i].absorption = newAbsorption;
						data.models[i].volume = newVolume;
						data.models[i].density = newDensity;
						data.models[i].metalness = newMetalness;

						data.models[i].updateTris(data);
					}
				}
			}

			ImGui::Separator();
		}

		if (ImGui::CollapsingHeader("Ray Marcher Material")) {
			ImGui::Separator();

			if (ImGui::ColorEdit3("rmAlbedo Color", (float*)&params.rmAlbedo, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoInputs)) {
				params.shouldSample = false;
			}

			if (ImGui::ColorEdit3("rmSpecular Color", (float*)&params.rmSpecularCol, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoInputs)) {
				params.shouldSample = false;
			}

			if (ImGui::ColorEdit3("rmEmission Color", (float*)&params.rmEmissionCol, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoInputs)) {
				params.shouldSample = false;
			}

			if (ImGui::ColorEdit3("rmAbsorption Color", (float*)&params.rmAbsorptionCol, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoInputs)) {
				params.shouldSample = false;
			}

			if (ImGui::ColorEdit3("rmVolume Color", (float*)&params.rmVolumeCol, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoInputs)) {
				params.shouldSample = false;
			}

			if (sliderHelper("rmIOR", "Index of Refraction of ray marcher material", sliderSize, params.rmIOR, 0.0f, 200.0f, LogSlider)) {
				params.shouldSample = false;
			}

			if (sliderHelper("rmRoughness", "Roughness of ray marcher material", sliderSize, params.rmRoughness, 0.0f, 1.0f)) {
				params.shouldSample = false;
			}

			if (sliderHelper("rmEmission Intensity", "Emission intensity of ray marcher material", sliderSize, params.rmEmissionIntensity, 0.0f, 100.0f, LogSlider)) {
				params.shouldSample = false;
			}

			if (sliderHelper("rmRefraction", "Refraction of ray marcher material", sliderSize, params.rmRefraction, 0.0f, 1.0f)) {
				params.shouldSample = false;
			}

			if (sliderHelper("rmAbsorption", "How much light a material absorbs when refracted", sliderSize, params.rmAbsorption, 0.0f, 10.0f)) {
				params.shouldSample = false;
			}

			if (sliderHelper("rmVolume", "Volume scattering of ray marcher material", sliderSize, params.rmVolume, 0.0f, 1.0f)) {
				params.shouldSample = false;
			}

			if (sliderHelper("rmDensity", "Volume density of ray marcher material", sliderSize, params.rmDensity, 0.0f, 100.0f)) {
				params.shouldSample = false;
			}

			if (sliderHelper("rmMetalness", "Metalness of ray marcher material", sliderSize, params.rmMetalness, 0.0f, 1.0f)) {
				params.shouldSample = false;
			}
		
			ImGui::Separator();
		}

		ImGui::Separator();
	}

	if (ImGui::CollapsingHeader("Debug Settings")) {
		ImGui::Separator();

		buttonHelper("Debug Ray", "Lets cast and see a ray by clicking on the scene", buttonSize, params.enableDebugRay);

		ImGui::Separator();
	}

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

	params.renderTime += params.dt;

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

	ImGui::Text("Render Time: %.2f sec", params.renderTime);

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

	if (ImGui::SliderFloat(("##" + label).c_str(), &parameter, minVal, maxVal, "%.4f")) {
		isSliderUsed = true;
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

	if (ImGui::SliderFloat(("##" + label).c_str(), &parameter, minVal, maxVal, "%.4f", ImGuiSliderFlags_Logarithmic)) {
		isSliderUsed = true;
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
	static std::unordered_map<ImGuiID, int> defaultValues;

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
