//
// Created by Michael Ankele on 2025-03-10.
//

#include "SphereMesh.h"
#include <data/mesh/GeometrySphere.h>

namespace artemis::graph {

void SphereMesh::on_process() {
	GeometrySphere geo(v_0, radius(), samples());
	geo.smoothen();
	out(geo);
}

} // graph