#pragma once
#include <glm/glm.hpp>
#include <fstream>
#include <sstream>
#include <memory>
#include <globalParams.h>

struct ObjImporter {

	float sceneScale = 1.0f;
	std::string fileName;

	//glm::vec3 col;
	float IOR;
	float roughness;
	float emissionIntensity;
	float refraction;
	float metalness;
	bool doubleSided;

	ObjImporter(
		std::string fileName,
		Data& data,
		float IOR,
		float roughness,
		float emissionIntensity,
		float refraction,
		float metalness,
		bool doubleSided
	) :
		fileName(fileName),
		IOR(IOR),
		roughness(roughness),
		emissionIntensity(emissionIntensity),
		refraction(refraction),
		metalness(metalness),
		doubleSided(doubleSided)
	{
		objImporter(data);
	}

	void objImporter(Data& data) {
		std::ifstream file(fileName);
		if (!file) {
			std::cerr << "Could not open file\n";
			return;
		}

		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> colors;
		std::vector<glm::vec3> normals;

		std::string line;

		while (std::getline(file, line)) {
			std::stringstream ss(line);
			std::string type;
			ss >> type;

			if (type == "v") {

				glm::vec3 v, c(0.7f);

				ss >> v.x >> v.y >> v.z;

				if (ss >> c.x >> c.y >> c.z) {}

				vertices.push_back(v);
				colors.push_back(c);
			}
			else if (type == "vn") {

				glm::vec3 n;
				ss >> n.x >> n.y >> n.z;

				normals.push_back(glm::normalize(n));
			}
			else if (type == "f") {

				std::vector<int> vertexIndices;
				std::vector<int> normalIndices;

				std::string token;

				while (ss >> token) {

					int vIdx = -1;
					int nIdx = -1;

					size_t firstSlash = token.find('/');
					size_t secondSlash = token.find('/', firstSlash + 1);

					if (firstSlash == std::string::npos) {
						vIdx = std::stoi(token);
					}
					else {
						vIdx = std::stoi(token.substr(0, firstSlash));

						if (secondSlash != std::string::npos &&
							secondSlash + 1 < token.size()) {
							nIdx = std::stoi(token.substr(secondSlash + 1));
						}
					}

					if (vIdx < 0) vIdx = (int)vertices.size() + vIdx;
					else          vIdx -= 1;

					if (nIdx < 0 && nIdx != -1)
						nIdx = (int)normals.size() + nIdx;
					else if (nIdx != -1)
						nIdx -= 1;

					vertexIndices.push_back(vIdx);
					normalIndices.push_back(nIdx);
				}

				for (size_t i = 1; i + 1 < vertexIndices.size(); ++i) {

					int iA = vertexIndices[0];
					int iB = vertexIndices[i];
					int iC = vertexIndices[i + 1];

					int nA = normalIndices[0];
					int nB = normalIndices[i];
					int nC = normalIndices[i + 1];

					if (iA < 0 || iB < 0 || iC < 0 ||
						iA >= (int)vertices.size() ||
						iB >= (int)vertices.size() ||
						iC >= (int)vertices.size()) {
						std::cerr << "Face index out of range\n";
						continue;
					}

					glm::vec3 avgColor =
						(colors[iA] + colors[iB] + colors[iC]) / 3.0f;

					glm::vec3 aN(0.0f);
					glm::vec3 bN(0.0f);
					glm::vec3 cN(0.0f);

					if (nA >= 0 && nA < normals.size())
						aN = normals[nA];

					if (nB >= 0 && nB < normals.size())
						bN = normals[nB];

					if (nC >= 0 && nC < normals.size())
						cN = normals[nC];

					data.tris.push_back({
						avgColor,
						{1.0f,1.0f,1.0f},
						avgColor,

						vertices[iA] * sceneScale,
						vertices[iB] * sceneScale,
						vertices[iC] * sceneScale,

						aN,
						bN,
						cN,

						IOR,
						roughness,
						emissionIntensity,
						refraction,
						metalness,
						doubleSided
						});
				}
			}
		}
	}

};