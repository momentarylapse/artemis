//
// Created by Michael Ankele on 2025-03-10.
//

#pragma once

#include "base.h"
#include "../grid/RegularGrid.h"

namespace artemis::data {

struct ScalarField {
	ScalarField();
	explicit ScalarField(const RegularGrid& grid, ScalarType type, SamplingMode sampling_mode);

	RegularGrid grid;
	ScalarType type;
	SamplingMode sampling_mode;
	SampledData<float, float> v32;
	SampledData<double, double> v64;

	double value(int i, int j, int k) const;
	void set(int i, int j, int k, double f);
	float value32(int i, int j, int k) const;
	void set32(int i, int j, int k, float f);
};

}

