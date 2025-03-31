//
// Created by Michael Ankele on 2025-03-11.
//

#pragma once

#include <lib/dataflow/Node.h>
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>
#include <data/field/ScalarField.h>
#include <data/mesh/PolygonMesh.h>

namespace artemis::graph {

class IsoSurface : public dataflow::Node {
public:
	IsoSurface() : Node("IsoSurface") {}

	void process() override;

	dataflow::Setting<float> iso_value{this, "iso-value", 0.0f};

	dataflow::InPort<artemis::data::ScalarField> in_field{this, "in"};
	dataflow::OutPort<PolygonMesh> out_mesh{this, "mesh"};
};

} // graph

