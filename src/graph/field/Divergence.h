//
// Created by Michael Ankele on 2025-03-14.
//

#pragma once

#include <data/field/ScalarField.h>
#include <graph/Node.h>
#include <graph/Port.h>
#include <data/field/VectorField.h>

namespace graph {

class Divergence : public Node {
public:
	Divergence() : Node("Divergence") {}

	void process() override;

	InPort<artemis::data::VectorField> in_field{this, "in"};

	OutPort<artemis::data::ScalarField> out{this, "out"};
};

} // graph
