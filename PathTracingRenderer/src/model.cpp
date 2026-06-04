#include "model.h"
#include "globalParams.h"

void PTModel::updateTris(Data& data) {
	for (size_t i = 0; i < tris.size(); i++) {

		Tri& t = data.tris[tris[i]];

		t.albedo = albedo;
		t.specularCol = specularCol;
		t.emissionCol = emissionCol;
		t.absorptionCol = absorptionCol;
		t.volumeCol = volumeCol;
		t.IOR = IOR;
		t.roughness = roughness;
		t.emissionIntensity = emissionIntensity;
		t.refraction = refraction;
		t.absorption = absorption;
		t.volume = volume;
		t.density = density;
		t.metalness = metalness;
		t.doubleSided = doubleSided;
	}
}
