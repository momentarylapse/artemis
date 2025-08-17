//
// Created by Michael Ankele on 2025-03-10.
//

#include "RegularGrid.h"
#include <data/grid/RegularGrid.h>

namespace artemis::graph {

void RegularGrid::on_process() {
	out(data::Grid(data::RegularGrid(nx(), ny(), nz(),
		vec3::EX * (float)lx() / (float)max(nx(), 1),
		vec3::EY * (float)ly() / (float)max(ny(), 1),
		vec3::EZ * (float)lz() / (float)max(nz(), 1))));
}


} // graph