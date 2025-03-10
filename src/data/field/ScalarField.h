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
	Array<float> v;

	float value(int i, int j, int k) const;
};

}

