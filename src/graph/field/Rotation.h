//
// Created by Michael Ankele on 2025-03-14.
//

#pragma once

#include <graph/Node.h>
#include <graph/Port.h>
#include <data/field/VectorField.h>
#include <graph/Setting.h>

namespace graph {

class Rotation : public Node {
public:
	Rotation() : Node("Rotation") {}

	void process() override;

	Setting<bool> backwards{this, "backwards", false};

	InPort<artemis::data::VectorField> in_field{this, "in"};

	OutPort<artemis::data::VectorField> out{this, "out"};
};

} // graph
