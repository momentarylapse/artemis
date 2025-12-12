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

class VectorFieldComponent : public dataflow::Node {
public:
	VectorFieldComponent() : Node("VectorFieldComponent") {}

	dataflow::Setting<int> component{this, "component", 0, "range=0:2"};

	void on_process() override;

	dataflow::InPort<artemis::data::VectorField> in_field{this, "in"};

	dataflow::OutPort<artemis::data::ScalarField> out{this, "out"};
};

} // graph