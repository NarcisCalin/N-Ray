#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <vector>
#include <raylib.h>
#include <fstream>
#include <sstream>
#include <memory>
#include <random>
#include <rlgl.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <globalParams.h>
#include <screenStartup.h>
#include <tri.h>
#include <bvh.h>
#include <renderer.h>
#include <camera.h>
#include <objImporter.h>

Params params;
Data data;
Screen screen{ float(params.screenSize.x), float(params.screenSize.y) };
PathTracer pt;
PTCam myCam;

std::vector<BVH> globalBVH;

void createFlatBVH() {

	globalBVH.clear();

	if (data.tris.empty()) {
		return;
	}

	globalBVH.reserve(data.tris.size() * 2);

	globalBVH.emplace_back();

	globalBVH[0] = BVH(0, static_cast<uint32_t>(data.tris.size() - 1), data.tris, globalBVH);
}

glm::mat4 rotation = glm::rotate(
	glm::mat4(1.0f),
	0.1f,
	glm::vec3(0.0f, 0.0f, 1.0f)
);

void mousePosDisplay() {

	Vector2 mousePos = GetMousePosition();

	DrawText(TextFormat("X: %d", int(mousePos.x)), int(mousePos.x) - 50, int(mousePos.y) - 40, 20, DARKGRAY);
	DrawText(TextFormat("Y: %d", int(mousePos.y)), int(mousePos.x) - 50, int(mousePos.y) - 20, 20, DARKGRAY);

}

struct AreaLight {

	glm::vec3 pos;
	glm::vec3 emissionCol;
	float emissionStrength;
	float size;

	Tri a;
	Tri b;

	AreaLight(glm::vec3 pos, glm::vec3 emissionCol, float emissionStrength, float size)

		: pos(pos),
		emissionCol(emissionCol),
		emissionStrength(emissionStrength),
		size(size),

		a{
			{0.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 0.0f},
			{emissionCol},

			{-size + pos.x, -size + pos.y, pos.z},
			{size + pos.x, size + pos.y, pos.z},
			{size + pos.x, -size + pos.y, pos.z},

			{0.0f, 0.0f, -1.0f},
			{0.0f, 0.0f, -1.0f},
			{0.0f, 0.0f, -1.0f},

			1.0f,
			1.0f,
			emissionStrength,
			0.0f,
			0.0f
		},

		b{
			{0.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 0.0f},
			{emissionCol},

			{size + pos.x, size + pos.y, pos.z},
			{-size + pos.x, -size + pos.y, pos.z},
			{-size + pos.x, size + pos.y, pos.z},

			{0.0f, 0.0f, -1.0f},
			{0.0f, 0.0f, -1.0f},
			{0.0f, 0.0f, -1.0f},

			1.0f,
			1.0f,
			emissionStrength,
			0.0f,
			0.0f
		} {

		addTrisToVector();
	}

	void addTrisToVector() {
		data.tris.push_back(a);
		data.tris.push_back(b);
	}
};

void createLights() {

	float lightHeight = 215.0f;

	AreaLight a{ {-57.0f, 0.0f, 215.0f}, { 1.0f, 1.0f, 1.0f }, 5.0f, 50.0f };
	AreaLight b{ {57.0f, 0.0f, 215.0f}, { 1.0f, 1.0f, 1.0f }, 5.0f, 50.0f };
}

void sortByEmission() {

	std::vector<std::pair<float, size_t>> triMap;

	for (size_t i = 0; i < data.tris.size(); i++) {

		float emVal = (data.tris[i].emissionCol.x + data.tris[i].emissionCol.y + data.tris[i].emissionCol.z) / 3.0f;

		float totalEmission = data.tris[i].emissionIntensity * emVal;

		triMap.push_back({ totalEmission, i });
	}

	std::sort(triMap.begin(), triMap.end(),
		[](const auto& a, const auto& b) {
			return a.first > b.first;
		}
	);

	std::vector<Tri> sortedTris;

	for (size_t i = 0; i < triMap.size(); i++) {
		sortedTris.push_back(data.tris[triMap[i].second]);
	}

	std::swap(sortedTris, data.tris);
}

void findEmissiveAmount() {
	for (size_t i = 0; i < data.tris.size(); i++) {

		float emVal = (data.tris[i].emissionCol.x + data.tris[i].emissionCol.y + data.tris[i].emissionCol.z) / 3.0f;

		float totalEmission = data.tris[i].emissionIntensity * emVal;

		if (totalEmission == 0.0f) {
			break;
		}

		params.emissiveAmount++;
	}
}

Camera3D cam3D;

PathRay mRay{ {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
Ray mouseRay = GetScreenToWorldRay({ 0.0f, 0.0f }, cam3D);

std::vector<DebugRay> debugRays;

float debugRaySpeed = 50.0f;

void traceDebugRay() {
	if (IsMouseButtonPressed(0)) {
		mouseRay = GetScreenToWorldRay(GetMousePosition(), cam3D);

		mRay.src = { mouseRay.position.x, mouseRay.position.y, mouseRay.position.z };
		mRay.dir = { mouseRay.direction.x, mouseRay.direction.y, mouseRay.direction.z };
		debugRays = pt.rayLogicDebug(mRay, data.tris, params);
	}

	for (size_t i = 0; i < debugRays.size(); i++) {

		DebugRay& r = debugRays[i];

		Color finalCol = {
			static_cast<unsigned char>(r.col.x * 255.0f),
			static_cast<unsigned char>(r.col.y * 255.0f),
			static_cast<unsigned char>(r.col.z * 255.0f),
			255
		};

		r.progress += GetFrameTime() * debugRaySpeed;

		if (debugRays[i - 1].progress < debugRays[i - 1].length && i > 0) {
			r.progress = 0.0f;
		}

		r.progress = std::min(r.progress, r.length);

		glm::vec3 length = r.dir * r.progress;

		glm::vec3 endPos = r.src + length;

		DrawCylinderEx({ r.src.x, r.src.y, r.src.z }, { endPos.x, endPos.y, endPos.z }, 0.01f, 0.01f, 12, finalCol);
	}
}

int main() {

	SetConfigFlags(FLAG_WINDOW_ALWAYS_RUN);
	SetTraceLogLevel(LOG_NONE);
	InitWindow(params.screenSize.x, params.screenSize.y, "Path Tracing");

	std::cout << "Loading Scene..." << '\n';
	ObjImporter scene{ "scene.obj", data, 1.5f, 1.0f, 0.0f, 0.0f, 0.0f };
	ObjImporter glass{ "sceneGlass.obj", data, 1.5f, 0.0f, 0.0f, 1.0f, 0.0f };
	ObjImporter metal{ "sceneMetal.obj", data, 1.5f, 0.0f, 0.0f, 0.0f, 1.0f };

	std::cout << "Creating Lights..." << '\n';
	//createLights();

	ObjImporter area{ "sceneArea.obj", data, 1.5f, 1.0f, 10.0f, 0.0f, 0.0f };

	std::cout << "Initializing Window..." << '\n';
	screen.initScreen();

	std::cout << "Build BVH Tree..." << '\n';
	createFlatBVH();

	//sortByEmission();
	//findEmissiveAmount();

	for (size_t i = 0; i < data.tris.size(); i++) {
		data.tris[i].idx = i;
	}

	cam3D.position = { myCam.camPos.x, myCam.camPos.y, myCam.camPos.z };
	cam3D.target = { myCam.camTarget.x, myCam.camTarget.y, myCam.camTarget.z };
	cam3D.up = { myCam.up.x, myCam.up.y, myCam.up.z };
	cam3D.fovy = myCam.fov;
	cam3D.projection = CAMERA_PERSPECTIVE;

	float totalMs = 0.0f;
	int totalFrames = 0;

	while (!WindowShouldClose()) {

		BeginDrawing();

		ClearBackground(BLACK);

		totalFrames++;

		params.dt = GetFrameTime();

		/*for (auto& tri : tris) {

			glm::vec4 va(
				tri.a.x,
				tri.a.y,
				tri.a.z,
				1.0f
			);

			va = rotation * va;

			tri.a.x = va.x;
			tri.a.y = va.y;
			tri.a.z = va.z;

			glm::vec4 vb(
				tri.b.x,
				tri.b.y,
				tri.b.z,
				1.0f
			);

			vb = rotation * vb;

			tri.b.x = vb.x;
			tri.b.y = vb.y;
			tri.b.z = vb.z;

			glm::vec4 vc(
				tri.c.x,
				tri.c.y,
				tri.c.z,
				1.0f
			);

			vc = rotation * vc;

			tri.c.x = vc.x;
			tri.c.y = vc.y;
			tri.c.z = vc.z;

			tri.calculateNormal();
			tri.calculateAABB();

		}

		bvh = createBVH();*/

		pt.render(data.rays, data.tris, myCam, screen, params);
		myCam.cameraLogic(params.dt);

		BeginMode3D(cam3D);

		//rlDisableBackfaceCulling();

		cam3D.position = { myCam.camPos.x, myCam.camPos.y, myCam.camPos.z };
		cam3D.target = { myCam.camTarget.x, myCam.camTarget.y, myCam.camTarget.z };
		cam3D.up = { myCam.up.x, myCam.up.y, myCam.up.z };

		float halfFovRadians = glm::radians(myCam.fov) * 0.5f;
		float actualMaxY = 0.5f * tan(halfFovRadians);
		float trueFovRadians = 2.0f * atan(actualMaxY);
		float raylibFov = glm::degrees(trueFovRadians);

		cam3D.fovy = raylibFov;

		/*for (size_t i = 0; i < data.tris.size(); i++) {

			Color finalcol = { unsigned char(data.tris[i].col.x * 255),
				unsigned char(data.tris[i].col.y * 255),
				unsigned char(data.tris[i].col.z * 255),
				255};

			DrawTriangle3D({ data.tris[i].a.x,data.tris[i].a.y,data.tris[i].a.z },
				{ data.tris[i].b.x,data.tris[i].b.y,data.tris[i].b.z },
				{ data.tris[i].c.x,data.tris[i].c.y,data.tris[i].c.z },
				finalcol);
		}

		for (size_t i = 0; i < globalBVH.size(); i++) {

			if (globalBVH[i].children[0] == UINT32_MAX || globalBVH[i].children[1] == UINT32_MAX) {
				continue;
			}

			BVH& na = globalBVH[globalBVH[i].children[0]];
			BVH& nb = globalBVH[globalBVH[i].children[1]];

			BoundingBox bba{ {na.min.x, na.min.y, na.min.z },{na.max.x, na.max.y, na.max.z } };
			BoundingBox bbb{ {nb.min.x, nb.min.y, nb.min.z },{nb.max.x, nb.max.y, nb.max.z } };

			DrawBoundingBox(bba, { 255, 0, 0, 100 });
			DrawBoundingBox(bbb, { 0, 255, 0, 100 });
		}*/

		traceDebugRay();

		EndMode3D();

		DrawFPS(10, 10);

		float ms = params.dt * 1000.0f;

		totalMs += ms;

		float avgMs = totalMs / float(totalFrames);

		DrawText(TextFormat("Frame time: %.2f ms", ms), 10, 40, 20, DARKGRAY);

		DrawText(TextFormat("Avg. Frame time: %.2f ms", avgMs), 10, 60, 20, DARKGRAY);

		DrawText(TextFormat("Triangles: %d", (int)data.tris.size()), 10, 80, 20, DARKGRAY);

		DrawText(TextFormat("Current Sample: %d", params.currentSample), 10, 100, 20, DARKGRAY);

		DrawText(TextFormat("Max Samples: %d", params.maxSamples), 10, 120, 20, DARKGRAY);

		DrawText(TextFormat("Bounces: %d", params.maxBounces), 10, 140, 20, DARKGRAY);

		DrawText(TextFormat("Total time (ms): %.2f ms", totalMs), 10, 160, 20, DARKGRAY);

		DrawText(TextFormat("Total time (sec): %.2f sec", totalMs / 1000.0f), 10, 180, 20, DARKGRAY);

		/*for (size_t i = 0; i < globalBVH.size(); i++) {

			if (i == selecIdx && i > 0) {

				FlatBVH& n = globalBVH[i];

				Vector2 screenPos = GetWorldToScreen({ n.max.x,n.max.y,n.max.z }, cam3D);

				DrawText(TextFormat("SelecIdx: %d", int(i)), (int)screenPos.x, (int)screenPos.y, 20, WHITE);
			}

			if (globalBVH[i].children[0] == UINT32_MAX || globalBVH[i].children[1] == UINT32_MAX) {
				continue;
			}

			FlatBVH& na = globalBVH[globalBVH[i].children[0]];
			FlatBVH& nb = globalBVH[globalBVH[i].children[1]];

			Vector2 screenPosA = GetWorldToScreen({ na.max.x,na.max.y,na.max.z }, cam3D);
			Vector2 screenPosB = GetWorldToScreen({ nb.max.x,nb.max.y,nb.max.z }, cam3D);

			DrawText(TextFormat("Idx: %d", int(globalBVH[i].children[0])), (int)screenPosA.x, (int)screenPosA.y, 15, RED);
			DrawText(TextFormat("Idx: %d", int(globalBVH[i].children[1])), (int)screenPosB.x, (int)screenPosB.y, 15, GREEN);
		}*/

		mousePosDisplay();

		EndDrawing();

		/*if (currentSample - 1 >= 1000) {
			TakeScreenshot("OutPerformance_.png");
			break;
		}*/
	}

	CloseWindow();
}