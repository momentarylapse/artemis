//
// Created by Michael Ankele on 2025-03-11.
//

#include "IsoSurface.h"
#include <processing/field/IsoSurface.h>

namespace graph {

void IsoSurface::process() {
	if (auto f = in_field.value()) {
		out_mesh(artemis::processing::iso_surface(*f, iso_value()));
	}
}


} // graph