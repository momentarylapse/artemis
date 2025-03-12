//
// Created by Michael Ankele on 2025-03-10.
//

#ifndef REGULARGRID_H
#define REGULARGRID_H

#include <lib/math/vec3.h>
#include <lib/math/Box.h>

namespace artemis::data {

struct RegularGrid {
	RegularGrid();
	RegularGrid(int nx, int ny, int nz, const vec3& dx = vec3::EX, const vec3& dy = vec3::EY, const vec3& dz = vec3::EZ);

	vec3 index_to_pos(int i, int j, int k) const;
	Box bounding_box() const;

	Array<vec3> grid_points() const;

	int nx, ny, nz;
	vec3 dx, dy, dz;
};

}


#endif //REGULARGRID_H
