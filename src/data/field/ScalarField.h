//
// Created by Michael Ankele on 2025-03-10.
//

#pragma once

#include "../grid/RegularGrid.h"

namespace artemis::data {

struct ScalarField {
	ScalarField();
	explicit ScalarField(const RegularGrid& grid);

	RegularGrid grid;
	Array<double> v;

	double value(int i, int j, int k) const;
	void set(int i, int j, int k, double f);
	float value32(int i, int j, int k) const;
	void set32(int i, int j, int k, float f);
};

}

