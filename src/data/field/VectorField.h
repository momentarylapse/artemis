//
// Created by Michael Ankele on 2025-03-11.
//

#pragma once

#include "../grid/RegularGrid.h"
#include <lib/math/vec3.h>

namespace artemis::data {

struct VectorField {
	VectorField();
	explicit VectorField(const RegularGrid& grid);

	RegularGrid grid;
	Array<vec3> v;

	vec3 value(int i, int j, int k) const;
	void set(int i, int j, int k, const vec3& v);
};

}


