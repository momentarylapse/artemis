//
// Created by Michael Ankele on 2025-03-10.
//

#include "ScalarField.h"

namespace artemis::data {

ScalarField::ScalarField(const RegularGrid& g) {
	grid = g;
	v.resize((grid.nx + 1) * (grid.ny + 1) * (grid.nz + 1));
}

ScalarField::ScalarField() : ScalarField(RegularGrid()) {}

float ScalarField::value(int i, int j, int k) const {
	return v[i + j * (grid.nx + 1) + k * (grid.nx + 1) * (grid.ny + 1)];
}

void ScalarField::set(int i, int j, int k, float f) {
	v[i + j * (grid.nx + 1) + k * (grid.nx + 1) * (grid.ny + 1)] = f;
}


}
