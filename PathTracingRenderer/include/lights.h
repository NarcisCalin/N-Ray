#pragma once
#include <tri.h>
#include <model.h>
#include <globalParams.h>

struct Data;

struct AreaLight {
    uint32_t triA;
    uint32_t triB;

    glm::vec3 pos;
    glm::vec3 emissionCol;
    glm::vec3 dir;
    float emissionStrength;
    float size;

    PTMaterial areaLightMat;

    AreaLight(glm::vec3 pos, glm::vec3 emissionCol, float emissionStrength, float size, Data& data);

    void addTrisToVector(Data& data);
};