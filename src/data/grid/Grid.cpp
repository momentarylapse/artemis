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

Box Grid::bounding_box() const {
	if (type == GridType::Regular)
		return regular->bounding_box();
	return {};
}

Array<vec3> Grid::points(SamplingMode mode) const {
	if (type == GridType::Regular)
		return regular->points(mode);
	return {};
}

Array<vec3> Grid::vertices() const {
	if (type == GridType::Regular)
		return regular->vertices();
	return {};
}

Array<vec3> Grid::cell_centers() const {
	if (type == GridType::Regular)
		return regular->cell_centers();
	return {};
}

Array<base::tuple<int, int> > Grid::edges() const {
	if (type == GridType::Regular)
		return regular->edges();
	return {};
}

int Grid::vertex_count() const {
	if (type == GridType::Regular)
		return regular->vertex_count();
	return 0;
}

int Grid::cell_count() const {
	if (type == GridType::Regular)
		return regular->cell_count();
	return 0;
}

}