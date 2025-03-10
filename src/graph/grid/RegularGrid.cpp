//
// Created by Michael Ankele on 2025-03-10.
//

#include "RegularGrid.h"

namespace graph {

void RegularGrid::process() {
	out(::RegularGrid(nx(), ny(), nz()));
	out_points(out.value->grid_points());
}


} // graph