//
// Created by michi on 09.03.25.
//

#include "TeapotMesh.h"
#include <data/mesh/GeometryTeapot.h>

namespace graph {

void TeapotMesh::process() {
	out_mesh(GeometryTeapot(v_0, radius(), samples()));
}

} // graph