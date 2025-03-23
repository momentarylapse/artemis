//
// Created by Michael Ankele on 2025-03-10.
//

#pragma once

#include "base.h"
#include "../grid/RegularGrid.h"

namespace artemis::data {

struct _ScalarField32 {
	using ScalarType = float;
	Array<float> v;
	void init(const RegularGrid& grid);
};

struct _ScalarField64 {
	using ScalarType = double;
	Array<double> v;
	void init(const RegularGrid& grid);
};

struct ScalarField {
	ScalarField();
	explicit ScalarField(const RegularGrid& grid, ScalarType type);

	RegularGrid grid;
	ScalarType type;
	_ScalarField32 v32;
	_ScalarField64 v64;

	double value(int i, int j, int k) const;
	void set(int i, int j, int k, double f);
	float value32(int i, int j, int k) const;
	void set32(int i, int j, int k, float f);
};

}

