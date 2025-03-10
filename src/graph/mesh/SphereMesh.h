//
// Created by Michael Ankele on 2025-03-10.
//

#ifndef SPHEREMESH_H
#define SPHEREMESH_H

#include "../Node.h"
#include "../Setting.h"
#include "../Port.h"
#include <data/mesh/PolygonMesh.h>

namespace graph {

class SphereMesh : public Node {
public:
	SphereMesh() : Node("SphereMesh") {}

	void process() override;

	Setting<float> radius{this, "radius", 1.0f};
	Setting<int> samples{this, "samples", 8};

	OutPort<PolygonMesh> out_mesh{this, "mesh"};
};

} // graph

#endif //SPHEREMESH_H
