//
// Created by Michael Ankele on 2025-03-11.
//

#include "IsoSurface.h"
#include <processing/field/IsoSurface.h>

namespace artemis::graph {

void IsoSurface::on_process() {
	if (auto f = in_field.value()) {
		out_mesh(artemis::processing::iso_surface(*f, (float)iso_value()));
	}
}


} // graph