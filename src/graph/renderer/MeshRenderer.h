//
// Created by michi on 09.03.25.
//

#ifndef MESHRENDERER_H
#define MESHRENDERER_H

#include "../Node.h"
#include "../Setting.h"
#include "../Port.h"
#include <data/mesh/PolygonMesh.h>

namespace graph {

class MeshRenderer : public Node {
public:
	MeshRenderer() : Node("MeshRenderer") {}

	InPort<PolygonMesh> in_mesh{this, "mesh"};
};

} // graph

#endif //MESHRENDERER_H
