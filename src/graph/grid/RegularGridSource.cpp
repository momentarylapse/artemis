//
// Created by Michael Ankele on 2025-03-10.
//

#include "RegularGridSource.h"

namespace graph {

void RegularGridSource::process() {
	out(RegularGrid(nx(), ny(), nz()));
}


} // graph