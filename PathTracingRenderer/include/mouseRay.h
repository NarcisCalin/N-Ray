#pragma once
#include <glm.hpp>
#include <raylib.h>
#include <screenStartup.h>
#include <globalParams.h>
#include <renderer.h>
#include <camera.h>
#include <random>

struct MouseRay {

	PathRay mouseRay(Params& params, Data& data, Screen& screen, PathTracer& pt, PTCam& myCam) {
		glm::vec2 mPos = { GetMousePosition().x / params.screenSize.x, GetMousePosition().y / params.screenSize.y };

		int x = int(screen.resX * mPos.x);
		int y = int(screen.resY * mPos.y);

		int mIdx = x + screen.resX * y;

		PathRay mRay;

		float srcOffsetX = (static_cast<float>(x) + 0.5f) / static_cast<float>(screen.resX) - 0.5f;
		float srcOffsetY = (static_cast<float>(y) + 0.5f) / static_cast<float>(screen.resY) - 0.5f;

		glm::vec3 src = myCam.camPos;

		src -= myCam.right * (srcOffsetX) * (myCam.sensorSize / 1000.0f);
		src += myCam.up * (srcOffsetY) * ((myCam.sensorSize / 1000.0f) / screen.ratio);

		glm::vec3 dir = glm::normalize(myCam.focalPoint - src);

		int index = y * screen.resX + x;

		mRay.src = src;
		mRay.dir = dir;
		//mRay.invDir = 1.0f / dir;

		return mRay;
	}

	PathRayState mouseRayState() {

		PathRayState mRayState;

		mRayState.active = true;
		mRayState.col = glm::vec3(0.0f);
		mRayState.throughput = glm::vec3(1.0f);

		return mRayState;
	}
};