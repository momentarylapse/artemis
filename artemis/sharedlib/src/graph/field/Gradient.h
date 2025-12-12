//
// Created by Michael Ankele on 2025-03-11.
//

#pragma once

#include <lib/dataflow/Node.h>
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>
#include <data/field/ScalarField.h>
#include <data/field/VectorField.h>

namespace artemis::graph {

class Gradient : public dataflow::Node {
public:
	Gradient() : Node("Gradient") {}

	void on_process() override;

	dataflow::InPort<artemis::data::ScalarField> in_field{this, "in"};

	dataflow::OutPort<artemis::data::VectorField> out{this, "out"};
};

} // graph
