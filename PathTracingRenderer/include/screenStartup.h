#pragma once

#include <glm/glm.hpp>
#include <raylib.h>

struct Pixel {
	glm::vec2 pos;
	float size;
	int x;
	int y;
	glm::vec3 color = { 0.0f, 0.0f, 0.0f };
	glm::vec3 accumColor = { 0.0f, 0.0f, 0.0f };

	Pixel(glm::vec2 pos, float size, int x, int y)
		: pos(pos), size(size), x(x), y(y) {
	}

	glm::vec3 contrastSCurve(glm::vec3 x, float c) {

		auto curve = [&](float v) -> float {
			if (v < 0.5f)
				return 0.5f * std::pow(2.0f * v, 1.0f + c);
			else
				return 1.0f - 0.5f * std::pow(2.0f * (1.0f - v), 1.0f + c);
			};
		return glm::clamp(glm::vec3(curve(x.r), curve(x.g), curve(x.b)), 0.0f, 1.0f);
	}

	void colorManagement(float c) {
		color = contrastSCurve(color, c);
		color = glm::pow(color, glm::vec3(1.0f / 2.2f));
	}

	void draw() {

		Color finalCol = { unsigned char(color.x * 255), unsigned char(color.y * 255), unsigned char(color.z * 255), 255 };
		DrawRectangleV({ pos.x, pos.y }, { size, size }, finalCol);
	}
};

struct Screen {

	std::vector<Pixel> pixels;

	float screenSizeX;
	float screenSizeY;

	Screen(float screenSizeX, float screenSizeY) :
		screenSizeX(screenSizeX), screenSizeY(screenSizeY)
	{
	}

	int res = 256;

	float ratio = screenSizeY / screenSizeX;

	int resX = res;
	int resY = float(res) * ratio;

	bool initGridFlag = true;

	float cellSize = screenSizeX / float(res) * 2.0f;

	void initScreen() {

		if (screenSizeX != GetScreenWidth() || screenSizeY != GetScreenHeight()) {
			initGridFlag = true;

			screenSizeX = float(GetScreenWidth());
			screenSizeY = float(GetScreenHeight());
		}

		if (initGridFlag) {

			pixels.clear();

			ratio = screenSizeX / screenSizeY;

			resX = res;
			resY = static_cast<int>(float(res) / ratio);

			cellSize = screenSizeX / float(resX);

			for (int y = 0; y < resY; y++) {
				for (int x = 0; x < resX; x++) {
					pixels.emplace_back(glm::vec2{ x * cellSize, y * cellSize }, cellSize, x, y);
				}
			}

			initGridFlag = false;
		}
	}

};
