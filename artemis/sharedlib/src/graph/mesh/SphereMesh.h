//
// Created by Michael Ankele on 2025-03-10.
//

#pragma once

#include <lib/dataflow/ResourceNode.h>
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>
#include <lib/mesh/PolygonMesh.h>

namespace artemis::graph {

class SphereMesh : public dataflow::ResourceNode<PolygonMesh> {
public:
	SphereMesh() : ResourceNode("SphereMesh") {}

	void on_process() override;

	dataflow::Setting<double> radius{this, "radius", 1.0};
	dataflow::Setting<int> samples{this, "samples", 8};
};

} // graph
