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

int RegularGrid::cell_count() const {
	return nx * ny * nz;
}

int RegularGrid::vertex_count() const {
	return (nx + 1) * (ny + 1) * (nz + 1);
}

int RegularGrid::cell_index(int i, int j, int k) const {
	return i + j * nx + k * nx * ny;
}

int RegularGrid::vertex_index(int i, int j, int k) const {
	return i + j * (nx + 1) + k * (nx + 1) * (ny + 1);
}


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
	//points.resize(vertex_count());
	for (int i=0; i<=nx; i++)
		for (int j=0; j<=ny; j++)
			for (int k=0; k<=nz; k++)
				points.add(vertex(i, j, k));
	return points;
}

Array<vec3> RegularGrid::cell_centers() const {
	Array<vec3> points;
	//points.resize(cell_count());
	for (int i=0; i<nx; i++)
		for (int j=0; j<ny; j++)
			for (int k=0; k<nz; k++)
				points.add(cell_center(i, j, k));
	return points;
}

}


