#include "lights.h"

AreaLight::AreaLight(glm::vec3 pos, glm::vec3 emissionCol, float emissionStrength, float size, Data& data)
    : pos(pos),
    emissionCol(emissionCol),
    dir(dir),
    emissionStrength(emissionStrength),
    size(size)
{
    areaLightMat = PTMaterial{
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
        emissionCol,
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
        1.0f,
        1.0f,
        emissionStrength,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f
    };

    addTrisToVector(data);
}

void AreaLight::addTrisToVector(Data& data) {

    data.models.push_back({ areaLightMat, false });

    triA = globalTriId;

	data.tris.push_back({ areaLightMat,
        {-size + pos.x, -size + pos.y, pos.z},
        { size + pos.x,  size + pos.y, pos.z},
        { size + pos.x, -size + pos.y, pos.z},
        {0.0f,0.0f,-0.0f},
        {0.0f,0.0f,-0.0f},
        {0.0f,0.0f,-0.0f},
        false });

    triB = globalTriId;

	data.tris.push_back({ areaLightMat,
        { size + pos.x,  size + pos.y, pos.z},
        {-size + pos.x, -size + pos.y, pos.z},
        {-size + pos.x,  size + pos.y, pos.z},
        {0.0f,0.0f,-0.0f},
        {0.0f,0.0f,-0.0f},
        {0.0f,0.0f,-0.0f},
        false });

    data.tris[triA].modelId = data.models.back().id;
    data.tris[triB].modelId = data.models.back().id;

    data.models.back().tris.push_back(triA);
    data.models.back().tris.push_back(triB);

    data.models.back().calculateOriginCenter(data);
}
