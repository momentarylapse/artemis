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
	SampledData<float> v32;
	SampledData<double> v64;

	double value(int index) const;
	void set(int index, double f);
	double _value(int i, int j, int k) const;
	void _set(int i, int j, int k, double f);
	float _value32(int i, int j, int k) const;
	void _set32(int i, int j, int k, float f);

	double average() const;
	double min() const;
	double max() const;

	void operator+=(const ScalarField& o);
	ScalarField operator+(const ScalarField& o) const;
	void operator-=(const ScalarField& o);
	ScalarField operator-(const ScalarField& o) const;
	void operator*=(double o);
	ScalarField operator*(double o) const;
};

}

