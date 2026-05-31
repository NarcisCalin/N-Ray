#include "model.h"
#include "globalParams.h"

void PTModel::updateTris(Data& data) {
	for (size_t i = 0; i < tris.size(); i++) {

		Tri& t = data.tris[tris[i]];

		t.albedo = albedo;
		t.specularCol = specularCol;
		t.emissionCol = emissionCol;
		t.refractionCol = refractionCol;
		t.IOR = IOR;
		t.roughness = roughness;
		t.emissionIntensity = emissionIntensity;
		t.refraction = refraction;
		t.metalness = metalness;
		t.doubleSided = doubleSided;
	}
}
