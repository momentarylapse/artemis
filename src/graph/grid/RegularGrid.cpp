//
// Created by Michael Ankele on 2025-03-10.
//

#include "RegularGrid.h"

namespace artemis::graph {

void RegularGrid::on_process() {
	out(data::RegularGrid(nx(), ny(), nz(),
		vec3::EX * (float)lx() / (float)max(nx(), 1),
		vec3::EY * (float)ly() / (float)max(ny(), 1),
		vec3::EZ * (float)lz() / (float)max(nz(), 1)));
	out_vertices(out.value->vertices());
	out_cell_centers(out.value->cell_centers());
}


} // graph