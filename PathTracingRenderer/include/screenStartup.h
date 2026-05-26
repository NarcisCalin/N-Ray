#pragma once

#include <glm/glm.hpp>
#include <raylib.h>

struct Screen {

	float screenSizeX;
	float screenSizeY;

	Screen(float screenSizeX, float screenSizeY) :
		screenSizeX(screenSizeX), screenSizeY(screenSizeY)
	{
	}

	float ratio = screenSizeX / screenSizeY;

	int resX;
	int resY;

	bool initGridFlag = true;

	void initScreen(int& res, std::vector<Color>& framebuffer, std::vector<glm::vec3>& accumBuffer) {

		if (screenSizeX != GetScreenWidth() || screenSizeY != GetScreenHeight()) {
			initGridFlag = true;

			screenSizeX = float(GetScreenWidth());
			screenSizeY = float(GetScreenHeight());
		}

		if (initGridFlag) {

			ratio = screenSizeX / screenSizeY;

			resX = res;
			resY = static_cast<int>(float(res) / ratio);

			framebuffer.resize(resX * resY);
			accumBuffer.resize(resX * resY);

			initGridFlag = false;
		}
	}

};
