#include "model.h"
#include "globalParams.h"

void PTModel::updateTris(Data& data) {
	for (size_t i = 0; i < tris.size(); i++) {

		Tri& t = data.tris[tris[i]];

		t.mat = mat;
		
		t.doubleSided = doubleSided;
	}
}
