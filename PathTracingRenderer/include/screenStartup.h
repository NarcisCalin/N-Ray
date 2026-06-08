#pragma once

#include <glm/glm.hpp>
#include <raylib.h>

struct Screen {

	float screenSizeX = 0.0f;
	float screenSizeY = 0.0f;

	float ratio = 0.0f;

	int resX = 0;
	int resY = 0;

	Screen(float screenSizeX, float screenSizeY) :
		screenSizeX(screenSizeX), screenSizeY(screenSizeY) {
	}

	void initScreen(int& res, std::vector<Color>& framebuffer, std::vector<glm::vec3>& accumBuffer, std::vector<glm::vec3>& causticsBuffer) {

		screenSizeX = float(GetScreenWidth());
		screenSizeY = float(GetScreenHeight());

		ratio = screenSizeX / screenSizeY;

		resX = res;
		resY = static_cast<int>(float(res) / ratio);

		framebuffer.resize(resX * resY);
		accumBuffer.resize(resX * resY);
		causticsBuffer.resize(resX * resY);
	}

};
