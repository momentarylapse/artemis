//
// Created by Michael Ankele on 2025-03-10.
//

#include "RegularGrid.h"

namespace artemis::graph {

void RegularGrid::process() {
	out(data::RegularGrid(nx(), ny(), nz(),
		vec3::EX * dx(),
		vec3::EY * dy(),
		vec3::EZ * dz()));
	out_vertices(out.value->vertices());
	out_cell_centers(out.value->cell_centers());
}


} // graph