//
// Created by michi on 09.03.25.
//

#include "TeapotMesh.h"
#include <lib/mesh/GeometryTeapot.h>

namespace artemis::graph {

void TeapotMesh::on_process() {
	auto geo = GeometryTeapot::create(v_0, (float)radius(), samples());
	geo.smoothen();
	out(geo);
}

} // graph