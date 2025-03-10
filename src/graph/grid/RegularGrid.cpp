//
// Created by Michael Ankele on 2025-03-10.
//

#include "RegularGrid.h"

namespace graph {

void RegularGrid::process() {
	out(artemis::data::RegularGrid(nx(), ny(), nz()));
	out_points(out.value->grid_points());
}


} // graph