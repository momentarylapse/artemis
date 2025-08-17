//
// Created by michi on 8/17/25.
//

#pragma once

#include "RegularGrid.h"
#include <lib/base/optional.h>
#include <lib/base/pointer.h>

namespace artemis::data {
enum class SamplingMode;

enum class GridType {
	None,
	Regular
};

struct Grid : Sharable<base::Empty> {
	Grid();
	explicit Grid(const RegularGrid& g);

	int vertex_count() const;
	int cell_count() const;
	Box bounding_box() const;

	Array<vec3> vertices() const;
	Array<vec3> cell_centers() const;
	Array<vec3> points(SamplingMode mode) const;

	Array<base::tuple<int, int>> edges() const;

	GridType type;
	base::optional<RegularGrid> regular;
};

}
