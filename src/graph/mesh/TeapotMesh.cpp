//
// Created by michi on 09.03.25.
//

#include "TeapotMesh.h"
#include <data/mesh/GeometryTeapot.h>

namespace artemis::graph {

void TeapotMesh::on_process() {
	GeometryTeapot geo(v_0, (float)radius(), samples());
	geo.smoothen();
	out(geo);
}

} // graph