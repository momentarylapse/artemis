//
// Created by Michael Ankele on 2025-03-10.
//

#include "RegularGrid.h"

namespace artemis::graph {

void RegularGrid::on_process() {
	out(data::RegularGrid(nx(), ny(), nz(),
		vec3::EX * (float)dx(),
		vec3::EY * (float)dy(),
		vec3::EZ * (float)dz()));
	out_vertices(out.value->vertices());
	out_cell_centers(out.value->cell_centers());
}


} // graph