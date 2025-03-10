//
// Created by Michael Ankele on 2025-03-10.
//

#include "ScalarField.h"

namespace artemis::data {

ScalarField::ScalarField(const RegularGrid& g) {
	grid = g;
	v.resize((grid.nx + 1) * (grid.ny + 1) * (grid.nz + 1));

	for (int i=0; i<v.num; i++)
		v[i] = pow(sin(i * 0.01f), 2);
}

ScalarField::ScalarField() : ScalarField(RegularGrid()) {}

float ScalarField::value(int i, int j, int k) const {
	return v[i + j * (grid.nx + 1) + k * (grid.nx + 1) * (grid.ny + 1)];
}


}
