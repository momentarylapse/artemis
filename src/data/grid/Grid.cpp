//
// Created by michi on 8/17/25.
//

#include "Grid.h"


namespace artemis::data {

Grid::Grid() {
	type = GridType::None;
}

Grid::Grid(const RegularGrid& g) {
	type = GridType::Regular;
	regular = g;
}

Grid::Grid(const UnstructuredGrid& g) {
	type = GridType::Unstructured;
	unstructured = g;
}

Box Grid::bounding_box() const {
	if (type == GridType::Regular)
		return regular->bounding_box();
	if (type == GridType::Unstructured)
		return unstructured->bounding_box();
	return {};
}

Array<vec3> Grid::points(SamplingMode mode) const {
	if (type == GridType::Regular)
		return regular->points(mode);
	if (type == GridType::Unstructured)
		return unstructured->points(mode);
	return {};
}

Array<vec3> Grid::vertices() const {
	if (type == GridType::Regular)
		return regular->vertices();
	if (type == GridType::Unstructured)
		return unstructured->vertices();
	return {};
}

Array<vec3> Grid::cell_centers() const {
	if (type == GridType::Regular)
		return regular->cell_centers();
	if (type == GridType::Unstructured)
		return unstructured->cell_centers();
	return {};
}

Array<base::tuple<int, int> > Grid::edges() const {
	if (type == GridType::Regular)
		return regular->edges();
	if (type == GridType::Unstructured)
		return unstructured->edges();
	return {};
}

int Grid::vertex_count() const {
	if (type == GridType::Regular)
		return regular->vertex_count();
	if (type == GridType::Unstructured)
		return unstructured->vertex_count();
	return 0;
}

int Grid::cell_count() const {
	if (type == GridType::Regular)
		return regular->cell_count();
	if (type == GridType::Unstructured)
		return unstructured->cell_count();
	return 0;
}

int Grid::count(SamplingMode mode) const {
	if (type == GridType::Regular)
		return regular->count(mode);
	if (type == GridType::Unstructured)
		return unstructured->count(mode);
	return 0;
}

}