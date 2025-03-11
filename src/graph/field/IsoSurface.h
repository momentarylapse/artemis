//
// Created by Michael Ankele on 2025-03-11.
//

#pragma once

#include <graph/Node.h>
#include <data/field/ScalarField.h>
#include <data/mesh/PolygonMesh.h>
#include <graph/Port.h>
#include <graph/Setting.h>

namespace graph {

class IsoSurface : public Node {
public:
	IsoSurface() : Node("IsoSurface") {}

	void process() override;

	Setting<float> iso_value{this, "iso-value", 0.0f};

	InPort<artemis::data::ScalarField> in_field{this, "in"};
	OutPort<PolygonMesh> out_mesh{this, "mesh"};
};

} // graph

