//
// Created by michi on 09.03.25.
//

#ifndef TEAPOTMESH_H
#define TEAPOTMESH_H

#include "../Node.h"
#include "../Setting.h"
#include "../Port.h"
#include <data/mesh/PolygonMesh.h>

namespace graph {

class TeapotMesh : public ResourceNode {
public:
	TeapotMesh() : ResourceNode("TeapotMesh") {}

	void process() override;

	Setting<float> radius{this, "radius", 1.0f};
	Setting<int> samples{this, "samples", 8};

	OutPort<PolygonMesh> out_mesh{this, "mesh"};
};

} // graph

#endif //TEAPOTMESH_H
