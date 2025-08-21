//
// Created by michi on 8/17/25.
//

#pragma once

#include "RegularGrid.h"
#include "UnstructuredGrid.h"
#include <lib/base/optional.h>
#include <lib/base/pointer.h>

namespace artemis::data {
enum class SamplingMode;

enum class GridType {
	None,
	Regular,
	Unstructured
};

struct Grid : Sharable<base::Empty> {
	Grid();
	explicit Grid(const RegularGrid& g);
	explicit Grid(const UnstructuredGrid& g);

	int vertex_count() const;
	int cell_count() const;
	int count(SamplingMode mode) const;
	Box bounding_box() const;

	Array<vec3> vertices() const;
	Array<vec3> cell_centers() const;
	Array<vec3> points(SamplingMode mode) const;

	Array<base::tuple<int, int>> edges() const;

	GridType type;
	base::optional<RegularGrid> regular;
	base::optional<UnstructuredGrid> unstructured;
};

}
