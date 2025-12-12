//
// Created by michi on 8/16/25.
//

#pragma once

#include <lib/dataflow/Node.h>
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>
#include <data/field/ScalarField.h>
#include <data/field/VectorField.h>

namespace artemis::graph {

class VectorFieldLength : public dataflow::Node {
public:
	VectorFieldLength() : Node("VectorFieldLength") {}

	void on_process() override;

	dataflow::InPort<artemis::data::VectorField> in_field{this, "in"};

	dataflow::OutPort<artemis::data::ScalarField> out{this, "out"};
};

} // graph
