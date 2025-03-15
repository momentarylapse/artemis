//
// Created by Michael Ankele on 2025-03-10.
//

#include "ScalarField.h"

namespace artemis::data {

ScalarField::ScalarField(const RegularGrid& g) {
	grid = g;
	v.resize(grid.nx * grid.ny * grid.nz);
}

ScalarField::ScalarField() : ScalarField(RegularGrid()) {}

double ScalarField::value(int i, int j, int k) const {
	return v[i + j * grid.nx + k * grid.nx * grid.ny];
}

void ScalarField::set(int i, int j, int k, double f) {
	v[i + j * grid.nx + k * grid.nx * grid.ny] = f;
}

float ScalarField::value32(int i, int j, int k) const {
	return (float)value(i, j, k);
}

void ScalarField::set32(int i, int j, int k, float f) {
	set(i, j, k, (double)f);
}


}
