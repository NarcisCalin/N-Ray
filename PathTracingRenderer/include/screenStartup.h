#pragma once

#include <glm/glm.hpp>
#include <raylib.h>
#include <globalParams.h>

struct Screen {

	float screenSizeX = 0.0f;
	float screenSizeY = 0.0f;

	float ratio = 0.0f;

	int resX = 0;
	int resY = 0;

	Screen(float screenSizeX, float screenSizeY) :
		screenSizeX(screenSizeX), screenSizeY(screenSizeY) {
	}

	void initScreen(int& res, Data& data) {

		screenSizeX = float(GetScreenWidth());
		screenSizeY = float(GetScreenHeight());

		ratio = screenSizeX / screenSizeY;

		resX = res;
		resY = static_cast<int>(float(res) / ratio);

		data.frameBuffer.resize(resX * resY);
		data.accumBuffer.resize(resX * resY);
		data.causticsBuffer.resize(resX * resY);
	}

};
