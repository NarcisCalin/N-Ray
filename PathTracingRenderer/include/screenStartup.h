#pragma once

#include <glm/glm.hpp>
#include <raylib.h>

struct Screen {

	float screenSizeX;
	float screenSizeY;

	float ratio;

	int resX;
	int resY;

	Screen(float screenSizeX, float screenSizeY) :
		screenSizeX(screenSizeX), screenSizeY(screenSizeY)
	{
	}

	void initScreen(int& res, std::vector<Color>& framebuffer, std::vector<glm::vec3>& accumBuffer) {

		screenSizeX = float(GetScreenWidth());
		screenSizeY = float(GetScreenHeight());

		ratio = screenSizeX / screenSizeY;

		resX = res;
		resY = static_cast<int>(float(res) / ratio);

		framebuffer.resize(resX * resY);
		accumBuffer.resize(resX * resY);
	}

};
