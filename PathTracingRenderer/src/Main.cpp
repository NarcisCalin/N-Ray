#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <vector>
#include <raylib.h>
#include <fstream>
#include <sstream>
#include <memory>
#include <random>
#include <rlgl.h>
#include <imgui.h>
#include <rlImGui.h>
#include <immintrin.h>

#define RAYGUI_IMPLEMENTATION

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <globalParams.h>
#include <screenStartup.h>
#include <tri.h>
#include <bvh.h>
#include <renderer.h>
#include <camera.h>
#include <objImporter.h>
#include <ui.h>
#include <mouseRay.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

Params params;
Data data;
Screen screen{ float(params.screenSize.x), float(params.screenSize.y) };
PathTracer pt;
PTCam myCam;
UI ui;
MouseRay mRayGen;

std::vector<BVH> globalBVH;
std::vector<CompactBVH> globalCompactBVH;

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
			{0.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 0.0f},

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
			0.0f,
			0.0f,
			0.0f,
			0.0f,
			false
		},

		b{
			{0.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 0.0f},
			{emissionCol},
			{0.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 0.0f},

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
			0.0f,
			0.0f,
			0.0f,
			0.0f,
			false
		} {

		addTrisToVector();
	}

	void addTrisToVector() {
		data.tris.push_back(a);
		data.tris.push_back(b);
	}
};

//void createLights() {
//
//	float lightHeight = 215.0f;
//
//	AreaLight a{ {-57.0f, 0.0f, 215.0f}, { 1.0f, 1.0f, 1.0f }, 5.0f, 50.0f };
//	AreaLight b{ {57.0f, 0.0f, 215.0f}, { 1.0f, 1.0f, 1.0f }, 5.0f, 50.0f };
//}

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

std::vector<DebugRay> debugRays;

float debugRaySpeed = 50.0f;

void traceDebugRay(Image& hdri) {
	if (IsMouseButtonPressed(0) && !params.isMouseHoveringUI && !myCam.clickDof) {

		PathRay mRay = mRayGen.mouseRay(params, data, screen, pt, myCam);
		PathRayState mRayState = mRayGen.mouseRayState();

		debugRays = pt.rayLogic(mRay, mRayState, data.tris, params, hdri, true);
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
		//DrawLine3D({ r.src.x, r.src.y, r.src.z }, { endPos.x, endPos.y, endPos.z }, finalCol);
	}
}

void setDofDist() {
	PathRay dofRay = mRayGen.mouseRay(params, data, screen, pt, myCam);
	PathRayState dofRayState = mRayGen.mouseRayState();

	float closestT = FLT_MAX;
	dofRayState.hit = false;
	dofRayState.triIdx = UINT32_MAX;

	pt.traverseFlatBVH(dofRay, dofRayState, closestT, data.tris, globalCompactBVH);

	if (!dofRayState.hit) {
		myCam.focusDist = closestT;
	}
	else {
		myCam.focusDist = glm::distance(myCam.camPos, dofRayState.hitPos);
	}
}

void selectModel() {
	if (IsMouseButtonPressed(0) && !params.isMouseHoveringUI) {
		PathRay selecRay = mRayGen.mouseRay(params, data, screen, pt, myCam);
		PathRayState selecRayState = mRayGen.mouseRayState();

		float closestT = FLT_MAX;
		selecRayState.hit = false;
		selecRayState.triIdx = UINT32_MAX;

		pt.traverseFlatBVH(selecRay, selecRayState, closestT, data.tris, globalCompactBVH);

		for (size_t i = 0; i < data.models.size(); i++) {
			data.models[i].selected = false;
		}

		if (selecRayState.hit) {
			data.models[data.tris[selecRayState.triIdx].modelIdx].selected = true;;
		}
	}
}

int main() {

	SetConfigFlags(FLAG_WINDOW_ALWAYS_RUN);
	SetTraceLogLevel(LOG_NONE);

	InitWindow(params.screenSize.x, params.screenSize.y, "Path Tracing");

	std::cout << "Loading Scene..." << '\n';
	ObjImporter scene{ "models/scene.obj", data,
		{0.7f, 0.7f, 0.7f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f},{0.0f, 0.0f, 0.0f},
		1.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, false };

	ObjImporter glass{ "models/sceneGlass.obj", data,
		{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f},{1.0f, 1.0f, 1.0f},{1.0f, 1.0f, 1.0f},
		1.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 15.0f, 0.0f , true };

	ObjImporter metal{ "models/sceneMetal.obj", data,
		{0.9f, 0.9f, 0.9f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f},{1.0f, 1.0f, 1.0f},{0.0f, 0.0f, 0.0f},
		1.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, true };

	ObjImporter red{ "models/sceneRed.obj", data,
		{0.7f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f},{1.0f, 1.0f, 1.0f},{0.0f, 0.0f, 0.0f},
		1.5f, 0.15f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, true };

	ObjImporter dragon{ "models/dragon.obj", data,
		{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f},{0.5f, 0.6f, 0.0f},{1.0f, 1.0f, 1.0f},
		1.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 15.0f, 0.0f , true };

	/*ObjImporter moon{ "models/moon.obj", data,
		{0.7f, 0.7f, 0.7f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f},{0.0f, 0.0f, 0.0f},
		1.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, false };*/

	std::cout << "Creating Lights..." << '\n';
	//createLights();

	//ObjImporter area{ "models/sceneArea.obj", data, 1.5f, 1.0f, 10.0f, 0.0f, 0.0f, false };
	//ObjImporter emissive{ "models/sceneEmissive.obj", data, 1.5f, 1.0f, 10.0f, 0.0f, 0.0f, true };

	std::cout << "Initializing Window..." << '\n';
	int prevRes = params.res;
	screen.initScreen(params.res, data.frameBuffer, data.accumBuffer);
	data.rays.resize(screen.resX * screen.resY);
	data.rayStates.resize(screen.resX * screen.resY);

	std::cout << "Build BVH Tree..." << '\n';
	createFlatBVH();
	if (!globalBVH.empty()) {
		pt.flattenBVH(0, globalBVH, globalCompactBVH);
	}

	/*sortByEmission();
	findEmissiveAmount();*/

	for (size_t i = 0; i < data.tris.size(); i++) {
		data.tris[i].idx = uint32_t(i);
		data.models[data.tris[i].modelIdx].tris.push_back(uint32_t(i));
	}

	std::cout << data.models.size();

	cam3D.position = { myCam.camPos.x, myCam.camPos.y, myCam.camPos.z };
	cam3D.target = { myCam.camTarget.x, myCam.camTarget.y, myCam.camTarget.z };
	cam3D.up = { myCam.up.x, myCam.up.y, myCam.up.z };
	cam3D.fovy = myCam.fov;
	cam3D.projection = CAMERA_PERSPECTIVE;

	Image ptData = {
		.data = data.frameBuffer.data(),
		.width = screen.resX,
		.height = screen.resY,
		.mipmaps = 1,
		.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
	};

	Texture2D render = LoadTextureFromImage(ptData);

	Image hdri = LoadImage("textures/HDRI.hdr");
	ImageFormat(&hdri, PIXELFORMAT_UNCOMPRESSED_R32G32B32);

	rlImGuiSetup(true);

	while (!WindowShouldClose()) {

		BeginDrawing();

		ClearBackground(BLACK);

		params.totalFrames++;

		params.dt = GetFrameTime();

		rlImGuiBegin();

		if (ImGui::IsAnyItemHovered() || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
			params.isMouseHoveringUI = true;
		}
		else {
			params.isMouseHoveringUI = false;
		}

		if (prevRes != params.res) {
			prevRes = params.res;
			screen.initScreen(params.res, data.frameBuffer, data.accumBuffer);
			data.rays.clear();

			UnloadTexture(render);
			Image ptData = {
				.data = data.frameBuffer.data(),
				.width = screen.resX,
				.height = screen.resY,
				.mipmaps = 1,
				.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
			};
			render = LoadTextureFromImage(ptData);

			params.shouldSample = false;

			data.rays.resize(screen.resX * screen.resY);
			data.rayStates.resize(screen.resX * screen.resY);
		}

		if (params.enableSampling && !params.isMouseHoveringUI) {
			params.shouldSample = true;
		}

		if (myCam.clickDof && IsMouseButtonPressed(0) && !params.isMouseHoveringUI) {
			setDofDist();
			params.shouldSample = false;
		}

		if (params.enableSelection) {
			selectModel();
		}

		myCam.cameraLogic(params, screen.ratio);

		cam3D.position = { myCam.camPos.x, myCam.camPos.y, myCam.camPos.z };
		cam3D.target = { myCam.camTarget.x, myCam.camTarget.y, myCam.camTarget.z };
		cam3D.up = { myCam.up.x, myCam.up.y, myCam.up.z };

		cam3D.fovy = myCam.fovV;

		if (params.render) {
			pt.render(data, myCam, screen, params, render, hdri);
		}

		BeginMode3D(cam3D);

		//rlDisableBackfaceCulling();

		if (params.enableDebugRay) {
			traceDebugRay(hdri);
		}

		if (!params.render) {
			rlBegin(RL_TRIANGLES);

			glm::vec3 camPos = myCam.camPos;

			auto fastClamp01 = [](float v) {
				return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
				};

			for (size_t i = 0; i < data.tris.size(); i++) {

				Tri& tri = data.tris[i];

				glm::vec3 lightDir = glm::normalize(tri.center - camPos);

				for (int j = 0; j < 3; j++) {

					glm::vec3 normal;
					glm::vec3 pos;

					if (j == 0) {
						normal = tri.aN;
						pos = tri.a;
					}
					else if (j == 1) {
						normal = tri.bN;
						pos = tri.b;
					}
					else {
						normal = tri.cN;
						pos = tri.c;
					}

					float light = fabs(glm::dot(lightDir, normal));

					float intensity = (light < 0.9f) ? (light * 0.9f) : (light * light);

					glm::vec3 col = tri.albedo * intensity;

					Color color = {
						(unsigned char)(fastClamp01(col.x) * 255),
						(unsigned char)(fastClamp01(col.y) * 255),
						(unsigned char)(fastClamp01(col.z) * 255),
						255
					};

					rlColor4ub(color.r, color.g, color.b, color.a);
					rlVertex3f(pos.x, pos.y, pos.z);
				}
			}

			rlEnd();
		}

		/*for (size_t i = 0; i < globalBVH.size(); i++) {

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

		EndMode3D();

		params.shouldSample = true;
		ui.logic(params, data, myCam);

		rlImGuiEnd();

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

		/*if (params.currentSample - 1 >= 1000) {
			TakeScreenshot("OutPerformance_.png");
			break;
		}*/
	}

	UnloadTexture(render);
	UnloadImage(hdri);

	CloseWindow();
}