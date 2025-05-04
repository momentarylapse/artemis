//
// Created by michi on 09.03.25.
//

#include "TeapotMesh.h"
#include <data/mesh/GeometryTeapot.h>

namespace artemis::graph {

void TeapotMesh::process() {
	GeometryTeapot geo(v_0, radius(), samples());
	geo.smoothen();
	out(geo);
}

} // graph