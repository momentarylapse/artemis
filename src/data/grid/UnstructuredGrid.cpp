//
// Created by michi on 8/21/25.
//

#include "UnstructuredGrid.h"
#include "data/field/base.h"
#include <lib/base/set.h>

namespace artemis::data {

int UnstructuredGrid::cell_count() const {
	return cells.num;
}

int UnstructuredGrid::vertex_count() const {
	return _vertices.num;;
}
int UnstructuredGrid::count(SamplingMode mode) const {
	if (mode == SamplingMode::PerCell)
		return cell_count();
	if (mode == SamplingMode::PerVertex)
		return vertex_count();
	return 0;
}


Box UnstructuredGrid::bounding_box() const {
	if (vertex_count() == 0)
		return {};
	Box box = {_vertices[0], _vertices[0]};
	for (const auto& v: _vertices)
		box = box or Box{v, v};
	return box;
}

Array<vec3> UnstructuredGrid::vertices() const {
	return _vertices;
}

Array<vec3> UnstructuredGrid::cell_centers() const {
	Array<vec3> points;
	points.simple_reserve(cell_count());
	for (const auto& c: cells)
		points.add(c.center(_vertices));
	return points;
}

vec3 UnstructuredGrid::Cell::center(const Array<vec3>& vertices) const {
	vec3 m = vec3(0,0,0);
	for (int i: indices)
		m += vertices[i];
	return m / (float)indices.num;
}

Array<base::tuple<int, int>> UnstructuredGrid::Cell::edges() const {
	Array<base::tuple<int, int>> r;
	if (type == Type::Line)
		return {{indices[0], indices[1]}};
	if (type == Type::Triangle)
		return {{indices[0], indices[1]}, {indices[1], indices[2]}, {indices[2], indices[0]}};
	if (type == Type::Polygon) {
		for (int k=0; k<indices.num; k++)
			r.add({indices[k], indices[(k+1) % indices.num]});
	}
	if (type == Type::Tetrahedron)
		return {
			{indices[0], indices[1]}, {indices[1], indices[2]}, {indices[2], indices[0]},
			{indices[0], indices[3]}, {indices[1], indices[3]}, {indices[2], indices[3]}};
	if (type == Type::Hexahedron)
		return {
			{indices[0], indices[1]}, {indices[1], indices[2]}, {indices[2], indices[3]}, {indices[3], indices[0]},
			{indices[4], indices[5]}, {indices[5], indices[6]}, {indices[6], indices[7]}, {indices[7], indices[4]},
			{indices[0], indices[4]}, {indices[1], indices[5]}, {indices[2], indices[6]}, {indices[3], indices[7]}};
	if (type == Type::TrianglePrism)
		return {
			{indices[0], indices[1]}, {indices[1], indices[2]}, {indices[2], indices[0]},
			{indices[3], indices[4]}, {indices[4], indices[5]}, {indices[5], indices[3]},
			{indices[0], indices[3]}, {indices[1], indices[4]}, {indices[2], indices[5]}};
	return r;
}



Array<vec3> UnstructuredGrid::points(SamplingMode mode) const {
	if (mode == SamplingMode::PerCell)
		return cell_centers();
	if (mode == SamplingMode::PerVertex)
		return vertices();
	return {};
}

Array<base::tuple<int, int>> UnstructuredGrid::edges() const {
	base::set<base::tuple<int, int>> edges;
	for (const auto& c: cells) {
		for (const auto& e: c.edges())
			edges.add({min(e.a, e.b), max(e.a, e.b)});
	}
	return edges;
}

}
