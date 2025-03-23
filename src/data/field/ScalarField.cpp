//
// Created by Michael Ankele on 2025-03-10.
//

#include "ScalarField.h"

namespace artemis::data {

void _ScalarField32::init(const RegularGrid& grid) {
	v.resize(grid.cell_count());
}

void _ScalarField64::init(const RegularGrid& grid) {
	v.resize(grid.cell_count());
}


ScalarField::ScalarField(const RegularGrid& g, ScalarType t) {
	grid = g;
	type = t;
	if (type == ScalarType::Float32)
		v32.init(grid);
	else if (type == ScalarType::Float64)
		v64.init(grid);
}

ScalarField::ScalarField() : ScalarField(RegularGrid(), ScalarType::None) {}

double ScalarField::value(int i, int j, int k) const {
	if (type == ScalarType::Float32)
		return (double)v32.v[grid.cell_index(i, j, k)];
	if (type == ScalarType::Float64)
		return v64.v[grid.cell_index(i, j, k)];
	return 0.0;
}

void ScalarField::set(int i, int j, int k, double f) {
	if (type == ScalarType::Float32)
		v32.v[grid.cell_index(i, j, k)] = (float)f;
	else if (type == ScalarType::Float64)
		v64.v[grid.cell_index(i, j, k)] = f;
}

float ScalarField::value32(int i, int j, int k) const {
	return (float)value(i, j, k);
}

void ScalarField::set32(int i, int j, int k, float f) {
	set(i, j, k, (double)f);
}


}
