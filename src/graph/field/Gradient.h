//
// Created by Michael Ankele on 2025-03-11.
//

#pragma once

#include <data/field/ScalarField.h>
#include <graph/Node.h>
#include <graph/Port.h>
#include <data/field/VectorField.h>

namespace graph {

class Gradient : public Node {
public:
	Gradient() : Node("Gradient") {}

	void process() override;

	InPort<artemis::data::ScalarField> in_field{this, "in"};

	OutPort<artemis::data::VectorField> out{this, "out"};
};

} // graph
