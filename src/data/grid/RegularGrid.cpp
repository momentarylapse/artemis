//
// Created by Michael Ankele on 2025-03-10.
//

#include "RegularGrid.h"

namespace artemis::data {

RegularGrid::RegularGrid(int _nx, int _ny, int _nz, const vec3& _dx, const vec3& _dy, const vec3& _dz) {
	nx = _nx;
	ny = _ny;
	nz = _nz;
	dx = _dx;
	dy = _dy;
	dz = _dz;
}

RegularGrid::RegularGrid() : RegularGrid(8, 8, 8) {}

vec3 RegularGrid::index_to_pos(float i, float j, float k) const {
	return i * dx + j * dy + k * dz;
}

vec3 RegularGrid::vertex(int i, int j, int k) const {
	return index_to_pos((float)i, (float)j, (float)k);
}

vec3 RegularGrid::cell_center(int i, int j, int k) const {
	return index_to_pos((float)i + 0.5f, (float)j + 0.5f, (float)k + 0.5f);
}


Box RegularGrid::bounding_box() const {
	// FIXME... :P
	return {v_0, vertex(nx, ny, nz)};
}

Array<vec3> RegularGrid::vertices() const {
	Array<vec3> points;
	//points.resize((nx + 1) * (ny + 1) * (nz + 1));
	for (int i=0; i<=nx; i++)
		for (int j=0; j<=ny; j++)
			for (int k=0; k<=nz; k++)
				points.add(vertex(i, j, k));
	return points;
}

Array<vec3> RegularGrid::cell_centers() const {
	Array<vec3> points;
	//points.resize(nx * ny * nz);
	for (int i=0; i<nx; i++)
		for (int j=0; j<ny; j++)
			for (int k=0; k<nz; k++)
				points.add(cell_center(i, j, k));
	return points;
}

}


