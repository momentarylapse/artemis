//
// Created by Michael Ankele on 2025-03-10.
//

#include "ScalarField.h"

namespace artemis::data {

ScalarField::ScalarField(const RegularGrid& g, ScalarType t, SamplingMode s) {
	grid = g;
	type = t;
	sampling_mode = s;
	if (type == ScalarType::Float32)
		v32.init(grid, sampling_mode);
	else if (type == ScalarType::Float64)
		v64.init(grid, sampling_mode);
}

ScalarField::ScalarField() : ScalarField(RegularGrid(), ScalarType::None, SamplingMode::PerCell) {}

double ScalarField::value(int i, int j, int k) const {
	if (type == ScalarType::Float32)
		return (double)v32.at(grid, sampling_mode, i, j, k);
	if (type == ScalarType::Float64)
		return v64.at(grid, sampling_mode, i, j, k);
	return 0.0;
}

void ScalarField::set(int i, int j, int k, double f) {
	if (type == ScalarType::Float32)
		v32.at(grid, sampling_mode, i, j, k) = (float)f;
	else if (type == ScalarType::Float64)
		v64.at(grid, sampling_mode, i, j, k) = f;
}

float ScalarField::value32(int i, int j, int k) const {
	return (float)value(i, j, k);
}

void ScalarField::set32(int i, int j, int k, float f) {
	set(i, j, k, (double)f);
}


}
