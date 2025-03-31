//
// Created by Michael Ankele on 2025-03-14.
//

#pragma once

#include <lib/dataflow/Node.h>
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>
#include <data/field/ScalarField.h>
#include <data/field/VectorField.h>

namespace artemis::graph {

class Divergence : public dataflow::Node {
public:
	Divergence() : Node("Divergence") {}

	void process() override;

	dataflow::InPort<artemis::data::VectorField> in_field{this, "in"};

	dataflow::OutPort<artemis::data::ScalarField> out{this, "out"};
};

} // graph
