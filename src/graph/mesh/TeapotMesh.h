//
// Created by michi on 09.03.25.
//

#ifndef TEAPOTMESH_H
#define TEAPOTMESH_H

#include <lib/dataflow/ResourceNode.h>
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>
#include <data/mesh/PolygonMesh.h>

namespace artemis::graph {

class TeapotMesh : public dataflow::ResourceNode<PolygonMesh> {
public:
	TeapotMesh() : ResourceNode("TeapotMesh") {}

	void process() override;

	dataflow::Setting<float> radius{this, "radius", 1.0f};
	dataflow::Setting<int> samples{this, "samples", 8};
};

} // graph

#endif //TEAPOTMESH_H
