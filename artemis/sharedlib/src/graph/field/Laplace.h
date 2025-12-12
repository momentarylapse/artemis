//
// Created by Michael Ankele on 2025-03-13.
//

#pragma once

#include <lib/dataflow/Node.h>
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>
#include <data/field/ScalarField.h>

namespace artemis::graph {

class Laplace : public dataflow::Node {
public:
	Laplace() : Node("Laplace") {}

	void on_process() override;

	dataflow::InPort<artemis::data::ScalarField> in_field{this, "in"};

	dataflow::OutPort<artemis::data::ScalarField> out{this, "out"};
};

} // graph
