//
// Created by Michael Ankele on 2025-03-10.
//

#include "RegularGrid.h"

namespace artemis::data {

RegularGrid::RegularGrid(int _nx, int _ny, int _nz) {
	nx = _nx;
	ny = _ny;
	nz = _nz;
	dx = vec3::EX;
	dy = vec3::EY;
	dz = vec3::EZ;
}

RegularGrid::RegularGrid() : RegularGrid(8, 8, 8) {}

vec3 RegularGrid::index_to_pos(int i, int j, int k) const {
	return (float)i * dx + (float)j * dy + (float)k * dz;
}

Box RegularGrid::bounding_box() const {
	// FIXME... :P
	return {v_0, index_to_pos(nx, ny, nz)};
}

Array<vec3> RegularGrid::grid_points() const {
	Array<vec3> points;
	//points.resize((nx + 1) * (nx + 1) * (nz * 1));
	for (int i=0; i<=nx; i++)
		for (int j=0; j<=ny; j++)
			for (int k=0; k<=nz; k++)
				points.add(index_to_pos(i, j, k));
	return points;
}

}


