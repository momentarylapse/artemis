//
// Created by michi on 8/21/25.
//

#pragma once

#pragma once

#include <lib/math/vec3.h>
#include <lib/math/Box.h>
#include <lib/base/tuple.h>

namespace artemis::data {
	enum class SamplingMode;

struct UnstructuredGrid {
	UnstructuredGrid() = default;

	int vertex_count() const;
	int cell_count() const;
	int count(SamplingMode mode) const;

	Box bounding_box() const;

	Array<vec3> vertices() const;
	Array<vec3> cell_centers() const;
	Array<vec3> points(SamplingMode mode) const;

	Array<base::tuple<int, int>> edges() const;

	Array<vec3> _vertices;

	struct Cell {
		enum class Type {
			Line,
			Triangle,
			Polygon,
			Tetrahedron,
			Hexahedron,
			TrianglePrism
		};
		Type type;
		Array<int> indices;
		vec3 center(const Array<vec3>& _vertices) const;
		Array<base::tuple<int, int>> edges() const;
	};
	Array<Cell> cells;
};

}

