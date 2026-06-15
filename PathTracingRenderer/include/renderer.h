#pragma once
#include <glm/glm.hpp>
#include <globalParams.h>
#include <camera.h>
#include <bvh.h>
#include <screenStartup.h>
#include <iostream>
#include <rtcore.h>
#include <glad/glad.h>

#include <fstream>
#include <sstream>
#include <string>

struct PathRay {
	glm::vec3 src;
	glm::vec3 dir;
};

struct PathRayState {
	glm::vec3 hitPos = { 0.0f, 0.0f, 0.0f };
	glm::vec3 col = { 0.0f, 0.0f, 0.0f };
	glm::vec3 causticsCol = { 0.0f, 0.0f, 0.0f };
	glm::vec3 throughput = { 1.0f, 1.0f, 1.0f };
	glm::vec3 rmNormal = { 0.0f, 0.0f, 0.0f };
	float length = FLT_MAX;
	uint32_t triIdx = UINT32_MAX;
	float rmMinLength = FLT_MAX;
	int rmSteps = 0;

	bool hit = false;
	bool active = true;
	bool isRefraction = false;
	bool isCaustic = false;
	bool isVolume = false;
};

struct DebugRay {
	glm::vec3 src;
	glm::vec3 dir;
	glm::vec3 col;
	float length;
	float progress = 0.0f;
};

struct Params;

struct PathTracer {

	bool rayIntersectsTriangle(PathRay& ray, const Tri& tri, float& t);

	//bool rayAABB(const PathRay& ray, const glm::vec3& boxMin, const glm::vec3& boxMax, float maxT);

	void flattenBVH(uint32_t buildNodeIdx, const std::vector<BVH>& buildNodes, std::vector<CompactBVH>& flatNodes);

	//void traverseFlatBVH(PathRay& ray, PathRayState& rayState, float& closestT, std::vector<Tri>& tris, const std::vector<CompactBVH>& flatBVH);

	void diffuseLighting(PathRay& ray, PathRayState& rayState, glm::vec3& normal, std::vector<Tri>& tris);

	void rmDiffuseLighting(PathRay& ray, PathRayState& rayState, glm::vec3& normal);

	const float airIOR = 1.0f;

	glm::vec3 sampleGGX(const glm::vec3& normal, float roughness, float r1, float r2);

	bool specularLighting(PathRay& ray, PathRayState& rayState, glm::vec3& normal, std::vector<Tri>& tris);

	bool rmSpecularLighting(PathRay& ray, PathRayState& rayState, glm::vec3& normal, float& roughness, float& IOR, float& metalness);

	void refractionLighting(PathRay& ray, PathRayState& rayState, glm::vec3 normal, std::vector<Tri>& tris);

	void rmRefractionLighting(PathRay& ray, PathRayState& rayState, glm::vec3 normal, float& IOR, float& absorption, glm::vec3& absorptionCol);

	bool traceRay(const EmbreeBVH& bvh, const PathRay& ray, PathRayState& rayState, float& closestT);

	//void directLight(glm::vec3& sampleP, glm::vec3& sampleN, float& pdf, Data& data, Params& params);

	glm::vec3 InterpolateNormal(PathRayState& rayState, std::vector<Tri>& tris);

	//std::vector<DebugRay> debugRays;

	//void sampleSun(PathRay& ray, std::vector<Tri>& tris, Params& params, bool& isShadow); // CURRENTLY UNUSED

	void causticsSampling(int& bounce, Data& data, Params& params, PathRay& ray, PathRayState& rayState, Image& hdri);

	glm::vec3 hdriLogic(PathRay& ray, Params& params);

	glm::vec3 environmentLogic(PathRay& ray, Params& params);

	void hdriBrigthestValue(Image& hdri, float& maxVal);

	void rayMarchingLogic(PathRay& ray, PathRayState& rayState, Params& params, Data& data, Image& hdri, PTCam& myCam);

	std::vector<DebugRay> rayLogic(PathRay& ray, PathRayState& rayState, Params& params, Data& data, Image& hdri, PTCam& myCam, bool debug = false);

	void rayGeneration(std::vector<PathRay>& rays, std::vector<PathRayState>& raysStates, PTCam& myCam, Screen& screen, Params& params);

	glm::vec3 contrastSCurve(glm::vec3 x, float c) {

		auto curve = [&](float v) -> float {
			if (v < 0.5f)
				return 0.5f * std::pow(2.0f * v, 1.0f + c);
			else
				return 1.0f - 0.5f * std::pow(2.0f * (1.0f - v), 1.0f + c);
			};
		return glm::clamp(glm::vec3(curve(x.r), curve(x.g), curve(x.b)), 0.0f, 1.0f);
	}

	void colorManagement(float c, glm::vec3& col) {
		col = contrastSCurve(col, c);
		col = glm::pow(col, glm::vec3(1.0f / 2.2f));
	}

	void drawScreen(Screen& screen, Params& params, Data& data, int& width, Texture2D& render);

	void render(Data& data, PTCam& myCam, Screen& screen, Params& params, Texture2D& render, Image& hdri);

	// GPU CODE STARTS HERE

	GLuint accumTextureID;
	Texture2D gpuAccumBuffer = { 0 };

	GLuint causticsTextureID;
	Texture2D gpuCausticsBuffer = { 0 };

	GLuint finalTextureID;
	Texture2D gpuFinalBuffer = { 0 };


	void createComputeTexture(int& resX, int& resY) {

		glGenTextures(1, &accumTextureID);
		glBindTexture(GL_TEXTURE_2D, accumTextureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, resX, resY);
		gpuAccumBuffer.id = accumTextureID;
		gpuAccumBuffer.width = resX;
		gpuAccumBuffer.height = resY;
		gpuAccumBuffer.mipmaps = 1;
		gpuAccumBuffer.format = PIXELFORMAT_UNCOMPRESSED_R32G32B32A32;

		SetTextureFilter(gpuAccumBuffer, TEXTURE_FILTER_POINT);

		glGenTextures(1, &causticsTextureID);
		glBindTexture(GL_TEXTURE_2D, causticsTextureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, resX, resY);
		gpuAccumBuffer.id = causticsTextureID;
		gpuAccumBuffer.width = resX;
		gpuAccumBuffer.height = resY;
		gpuAccumBuffer.mipmaps = 1;
		gpuAccumBuffer.format = PIXELFORMAT_UNCOMPRESSED_R32G32B32A32;

		SetTextureFilter(gpuCausticsBuffer, TEXTURE_FILTER_POINT);

		glGenTextures(1, &finalTextureID);
		glBindTexture(GL_TEXTURE_2D, finalTextureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, resX, resY);
		gpuFinalBuffer.id = finalTextureID;
		gpuFinalBuffer.width = resX;
		gpuFinalBuffer.height = resY;
		gpuFinalBuffer.mipmaps = 1;
		gpuFinalBuffer.format = PIXELFORMAT_UNCOMPRESSED_R32G32B32A32;

		SetTextureFilter(gpuFinalBuffer, TEXTURE_FILTER_POINT);
	}

	std::string loadShader(const std::string& path) {

		std::ifstream file(path);
		if (!file.is_open())
			throw std::runtime_error("Failed to open shader file: " + path);

		std::ostringstream ss;
		ss << file.rdbuf();
		return ss.str();
	}

	GLuint compileComputeShader(const std::string& path) {

		std::string sourceStr = loadShader(path);
		const char* source = sourceStr.c_str();

		GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(shader, 1, &source, NULL);
		glCompileShader(shader);

		GLint success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetShaderInfoLog(shader, 512, NULL, infoLog);
			std::cerr << "ERROR::SHADER::COMPUTE::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		GLuint program = glCreateProgram();
		glAttachShader(program, shader);
		glLinkProgram(program);

		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetProgramInfoLog(program, 512, NULL, infoLog);
			std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}

		glDeleteShader(shader);
		return program;
	}

	GLuint computeProgram;

	// Global Params
	GLint timeLoc;
	GLint blurLoc;

	GLint screenRatioLoc;
	GLint resXLoc;
	GLint resYLoc;

	GLint currentSampleLoc;
	GLint maxSamplesLoc;
	GLint shouldSampleLoc;

	GLint rayMarcherLoc;
	GLint pathTracerLoc;

	GLint enableCausticsLoc;

	GLint skyIntensityLoc;
	GLint enableSkyLoc;
	GLint skyTopLoc;
	GLint skyBaseLoc;

	GLint enableHDRILoc;

	GLint enableSunLoc;
	GLint sunAngleLoc;
	GLint sunColorLoc;
	GLint sunIntensityLoc;
	GLint sunDirLoc;

	GLint bouncesLoc;

	GLint raysPerPixelLoc;

	GLint hdriRotationLoc;
	GLint hdriBrigthestLoc;
	GLint hdriThresholdLoc;

	// Ray Marching Params
	GLint rmMaxStepsLoc;
	GLint rmNearPlaneLoc;
	GLint rmFarPlaneLoc;
	GLint rmPBRLoc;

	GLint rmLodMinDistMultLoc;
	GLint rmLodAmountLoc;

	// Camera Params
	GLint apertureLoc;
	GLint camPosLoc;

	GLint rightLoc;
	GLint upLoc;
	GLint camNormalLoc;
	GLint sensorSizeLoc;
	GLint ISOLoc;
	GLint focalPointLoc;
	GLint focusDistLoc;

	// Scene Data
	GLuint triangleSSBO = 0;
	GLuint bvhSSBO = 0;
	GLint nodeCountLoc;

	// Image Params
	GLint mainBufferIntensityLoc;
	GLint causticsIntensityLoc;

	GLint exposureLoc;
	GLint contrastLoc;

	GLuint hdriLoc;
	GLuint hdrUnit = 3;

	Texture2D hdriTex;

	GLuint materialUBO = 0;

	void loadHDRIToGPU(Params& params) {
		hdriTex = LoadTextureFromImage(params.hdri);
		SetTextureFilter(hdriTex, TEXTURE_FILTER_BILINEAR);
	}

	void initTriangleSSBO(const std::vector<TriGPU>& trisGPU) {

		glGenBuffers(1, &triangleSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangleSSBO);

		glBufferData(
			GL_SHADER_STORAGE_BUFFER,
			trisGPU.size() * sizeof(TriGPU),
			trisGPU.data(),
			GL_STATIC_DRAW
		);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, triangleSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void initBVHSSBO(const std::vector<CompactBVHGPU>& globalCompactBVHGPU) {

		glGenBuffers(1, &bvhSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, bvhSSBO);

		glBufferData(
			GL_SHADER_STORAGE_BUFFER,
			globalCompactBVHGPU.size() * sizeof(CompactBVHGPU),
			globalCompactBVHGPU.data(),
			GL_STATIC_DRAW
		);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, bvhSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void uploadTriangles(Data& data) {
		data.trisGPU.resize(data.tris.size());

		for (size_t i = 0; i < data.tris.size(); i++) {
			const Tri& tri = data.tris[i];
			TriGPU& gpuTri = data.trisGPU[i];

			const PTMaterial& m = tri.mat;

			gpuTri.mat = PTMaterialGPU(
				m.albedo, m.roughness,
				m.specularCol, m.metalness,
				m.emissionCol, m.emissionIntensity,
				m.absorptionCol, m.absorption,
				m.volumeCol, m.density,
				m.IOR, m.refraction, m.volume
			);

			gpuTri.a = glm::vec4(tri.a, 1.0f);
			gpuTri.b = glm::vec4(tri.b, 1.0f);
			gpuTri.c = glm::vec4(tri.c, 1.0f);

			gpuTri.aN = glm::vec4(tri.aN, 0.0f);
			gpuTri.bN = glm::vec4(tri.bN, 0.0f);
			gpuTri.cN = glm::vec4(tri.cN, 0.0f);

			gpuTri.eA = glm::vec4(tri.eA, 0.0f);
			gpuTri.eB = glm::vec4(tri.eB, 0.0f);

			gpuTri.normal = glm::vec4(tri.normal, 0.0f);

			gpuTri.min = glm::vec4(tri.min, 0.0f);
			gpuTri.max = glm::vec4(tri.max, 0.0f);
			gpuTri.center = glm::vec4(tri.center, 0.0f);

			gpuTri.id = tri.id;
			gpuTri.doubleSided = tri.doubleSided ? 1u : 0u;
			gpuTri.modelId = tri.modelId;
			gpuTri.padding = 0;
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangleSSBO);

		glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
		glBufferData(GL_SHADER_STORAGE_BUFFER,
			data.trisGPU.size() * sizeof(TriGPU),
			data.trisGPU.data(),
			GL_DYNAMIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void initCompute(int& resX, int& resY) {

		createComputeTexture(resX, resY);

		computeProgram = compileComputeShader("shaders/ptShader.comp");

		// Global Params
		timeLoc = glGetUniformLocation(computeProgram, "time");
		blurLoc = glGetUniformLocation(computeProgram, "blur");

		screenRatioLoc = glGetUniformLocation(computeProgram, "screenRatio");
		resXLoc = glGetUniformLocation(computeProgram, "resX");
		resYLoc = glGetUniformLocation(computeProgram, "resY");

		currentSampleLoc = glGetUniformLocation(computeProgram, "currentSample");
		maxSamplesLoc = glGetUniformLocation(computeProgram, "maxSamples");
		shouldSampleLoc = glGetUniformLocation(computeProgram, "shouldSample");

		rayMarcherLoc = glGetUniformLocation(computeProgram, "rayMarcher");
		pathTracerLoc = glGetUniformLocation(computeProgram, "pathTracer");

		enableCausticsLoc = glGetUniformLocation(computeProgram, "enableCaustics");

		skyIntensityLoc = glGetUniformLocation(computeProgram, "skyIntensity");
		enableSkyLoc = glGetUniformLocation(computeProgram, "enableSky");
		skyTopLoc = glGetUniformLocation(computeProgram, "skyTop");
		skyBaseLoc = glGetUniformLocation(computeProgram, "skyBase");

		enableHDRILoc = glGetUniformLocation(computeProgram, "enableHDRI");

		enableSunLoc = glGetUniformLocation(computeProgram, "enableSun");
		sunAngleLoc = glGetUniformLocation(computeProgram, "sunAngle");
		sunColorLoc = glGetUniformLocation(computeProgram, "sunColor");
		sunIntensityLoc = glGetUniformLocation(computeProgram, "sunIntensity");

		sunDirLoc = glGetUniformLocation(computeProgram, "sunDir");

		bouncesLoc = glGetUniformLocation(computeProgram, "maxBounces");

		mainBufferIntensityLoc = glGetUniformLocation(computeProgram, "mainBufferIntensity");
		causticsIntensityLoc = glGetUniformLocation(computeProgram, "causticsIntensity");

		raysPerPixelLoc = glGetUniformLocation(computeProgram, "raysPerPixel");

		hdriRotationLoc = glGetUniformLocation(computeProgram, "hdriRotation");
		hdriBrigthestLoc = glGetUniformLocation(computeProgram, "hdriBrigthest");
		hdriThresholdLoc = glGetUniformLocation(computeProgram, "hdriThreshold");

		// Ray Marching Params
		rmMaxStepsLoc = glGetUniformLocation(computeProgram, "rmMaxSteps");
		rmNearPlaneLoc = glGetUniformLocation(computeProgram, "rmNearPlane");
		rmFarPlaneLoc = glGetUniformLocation(computeProgram, "rmFarPlane");
		rmPBRLoc = glGetUniformLocation(computeProgram, "rmPBR");

		rmLodMinDistMultLoc = glGetUniformLocation(computeProgram, "rmLodMinDistMult");
		rmLodAmountLoc = glGetUniformLocation(computeProgram, "rmLodAmount");

		glGenBuffers(1, &materialUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, materialUBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(PTMaterialGPU), nullptr, GL_DYNAMIC_DRAW);

		glBindBufferBase(GL_UNIFORM_BUFFER, 5, materialUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		// Camera Params
		apertureLoc = glGetUniformLocation(computeProgram, "aperture");
		camPosLoc = glGetUniformLocation(computeProgram, "camPos");

		rightLoc = glGetUniformLocation(computeProgram, "right");
		upLoc = glGetUniformLocation(computeProgram, "up");
		camNormalLoc = glGetUniformLocation(computeProgram, "camNormal");

		sensorSizeLoc = glGetUniformLocation(computeProgram, "sensorSize");
		ISOLoc = glGetUniformLocation(computeProgram, "ISO");

		focalPointLoc = glGetUniformLocation(computeProgram, "focalPoint");
		focusDistLoc = glGetUniformLocation(computeProgram, "focusDist");

		// Scene Params
		nodeCountLoc = glGetUniformLocation(computeProgram, "nodeCount");
		hdriLoc = glGetUniformLocation(computeProgram, "hdri");

		// Image Params
		exposureLoc = glGetUniformLocation(computeProgram, "exposure");
		contrastLoc = glGetUniformLocation(computeProgram, "contrast");

	}

	void gpuSampleLogic(Params& params) {
		if (!params.shouldSample) {

			params.currentSample = 1;
			params.renderTime = 0.0f;
		}

		if (!params.shouldSample || params.currentSample < params.maxSamples) {

			if (params.shouldSample) {
				params.currentSample++;
			}
		}
	}

	void updateRMMaterialGPU(Params& params) {
		params.rmMatGPU.albedo_roughness = { params.rmMat.albedo, params.rmMat.roughness };
		params.rmMatGPU.specular_metalness = { params.rmMat.specularCol, params.rmMat.metalness };
		params.rmMatGPU.emission_intensity = { params.rmMat.emissionCol, params.rmMat.emissionIntensity };
		params.rmMatGPU.absorption_amount = { params.rmMat.absorptionCol, params.rmMat.absorption };
		params.rmMatGPU.volume_density = { params.rmMat.volumeCol, params.rmMat.density };
		params.rmMatGPU.extraParams = { params.rmMat.IOR, params.rmMat.refraction, params.rmMat.volume, 0.0f };
	}

	void runShader(glm::ivec2 screenSize, int& resX, int& resY, PTCam& myCam, Params& params, Screen& screen) {

		glUseProgram(computeProgram);

		// Global Params
		glUniform1f(timeLoc, params.totalMs);
		glUniform1f(blurLoc, params.blur);

		glUniform1f(screenRatioLoc, screen.ratio);
		glUniform1i(resXLoc, screen.resX);
		glUniform1i(resYLoc, screen.resY);

		gpuSampleLogic(params);

		glUniform1i(currentSampleLoc, params.currentSample);
		glUniform1i(maxSamplesLoc, params.maxSamples);
		glUniform1i(shouldSampleLoc, params.shouldSample);

		glUniform1i(rayMarcherLoc, params.rayMarcher);
		glUniform1i(pathTracerLoc, params.pathTracer);

		glUniform1i(enableCausticsLoc, params.enableCaustics);

		glUniform1f(skyIntensityLoc, params.skyIntensity);

		glUniform1i(enableSkyLoc, params.enableSky);

		glUniform3f(skyTopLoc,params.skyTop.x,params.skyTop.y,params.skyTop.z);

		glUniform3f(skyBaseLoc,params.skyBase.x,params.skyBase.y,params.skyBase.z);

		glUniform1i(enableHDRILoc, params.enableHDRI);

		glUniform1i(enableSunLoc, params.enableSun);

		glUniform1f(sunAngleLoc, params.sunAngle);

		glUniform3f(sunColorLoc,params.sunColor.x,params.sunColor.y,params.sunColor.z);

		glUniform1f(sunIntensityLoc, params.sunIntensity);

		glUniform3f(sunDirLoc, params.sunDir.x, params.sunDir.y, params.sunDir.z);

		glUniform1i(bouncesLoc, params.maxBounces);

		glUniform1i(raysPerPixelLoc, params.raysPerPixel);

		glUniform1f(hdriRotationLoc, params.hdriRotation);
		glUniform1f(hdriBrigthestLoc, params.hdriBrigthest);
		glUniform1f(hdriThresholdLoc, params.hdriThreshold);

		// Ray Marching Params
		glUniform1i(rmMaxStepsLoc, params.rmMaxSteps);
		glUniform1f(rmNearPlaneLoc, params.rmNearPlane);
		glUniform1f(rmFarPlaneLoc, params.rmFarPlane);
		glUniform1i(rmPBRLoc, params.rmPBR);

		glUniform1f(rmLodMinDistMultLoc, params.rmLodMinDistMult);
		glUniform1f(rmLodAmountLoc, params.rmLodAmount);

		updateRMMaterialGPU(params);
		glBindBuffer(GL_UNIFORM_BUFFER, materialUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PTMaterialGPU), &params.rmMatGPU);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		// Camera Params
		glUniform1f(apertureLoc, myCam.aperture);

		glUniform3f(camPosLoc, myCam.camPos.x, myCam.camPos.y, myCam.camPos.z);

		glUniform3f(rightLoc, myCam.right.x, myCam.right.y, myCam.right.z);

		glUniform3f(upLoc, myCam.up.x, myCam.up.y, myCam.up.z);

		glUniform3f(camNormalLoc, myCam.camNormal.x, myCam.camNormal.y, myCam.camNormal.z);

		glUniform1f(sensorSizeLoc, myCam.sensorSize);
		glUniform1f(ISOLoc, myCam.ISO);

		glUniform3f(focalPointLoc, myCam.focalPoint.x, myCam.focalPoint.y, myCam.focalPoint.z);
		glUniform1f(focusDistLoc, myCam.focusDist);

		// Scene Data
		glUniform1ui(nodeCountLoc, uint32_t(globalCompactBVHGPU.size()));

		glActiveTexture(GL_TEXTURE0 + hdrUnit);
		glBindTexture(GL_TEXTURE_2D, hdriTex.id);
		glUniform1i(hdriLoc, hdrUnit);

		// Image Params
		glUniform1f(mainBufferIntensityLoc, params.mainBufferIntensity);
		glUniform1f(causticsIntensityLoc, params.causticsIntensity);

		glUniform1f(exposureLoc, params.exposure);
		glUniform1f(contrastLoc, params.contrast);

		glBindImageTexture(0, accumTextureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glBindImageTexture(1, finalTextureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glBindImageTexture(2, causticsTextureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

		GLuint numGroupsX = (resX + 15) / 16;
		GLuint numGroupsY = (resY + 15) / 16;

		glDispatchCompute(numGroupsX, numGroupsY, 1);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		DrawTexturePro(
			gpuFinalBuffer,
			Rectangle({ 0.0f, 0.0f, float(gpuFinalBuffer.width), float(gpuFinalBuffer.height) }),
			Rectangle({ 0.0f, 0.0f, float(screenSize.x), float(screenSize.y) }),
			{ 0.0f, 0.0f },
			0.0f,
			WHITE
		);
	}
};