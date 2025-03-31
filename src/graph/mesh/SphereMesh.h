//
// Created by Michael Ankele on 2025-03-10.
//

#ifndef SPHEREMESH_H
#define SPHEREMESH_H

#include <lib/dataflow/Node.h>
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>
#include <data/mesh/PolygonMesh.h>

namespace artemis::graph {

class SphereMesh : public dataflow::ResourceNode {
public:
	SphereMesh() : ResourceNode("SphereMesh") {}

	void process() override;

	dataflow::Setting<float> radius{this, "radius", 1.0f};
	dataflow::Setting<int> samples{this, "samples", 8};

	dataflow::OutPort<PolygonMesh> out_mesh{this, "mesh"};
};

} // graph

#endif //SPHEREMESH_H
