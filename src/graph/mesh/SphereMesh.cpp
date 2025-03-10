//
// Created by Michael Ankele on 2025-03-10.
//

#include "SphereMesh.h"
#include <data/mesh/GeometrySphere.h>

namespace graph {

void SphereMesh::process() {
	out_mesh(GeometrySphere(v_0, radius(), samples()));
	dirty = false;
}

} // graph