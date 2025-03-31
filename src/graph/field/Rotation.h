//
// Created by Michael Ankele on 2025-03-14.
//

#pragma once

#include <lib/dataflow/Node.h>
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>
#include <data/field/VectorField.h>

namespace artemis::graph {

class Rotation : public dataflow::Node {
public:
	Rotation() : Node("Rotation") {}

	void process() override;

	dataflow::Setting<bool> backwards{this, "backwards", false};

	dataflow::InPort<artemis::data::VectorField> in_field{this, "in"};

	dataflow::OutPort<artemis::data::VectorField> out{this, "out"};
};

} // graph
