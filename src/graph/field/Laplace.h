//
// Created by Michael Ankele on 2025-03-13.
//

#pragma once

#include <data/field/ScalarField.h>
#include <graph/Node.h>
#include <graph/Port.h>

namespace graph {

class Laplace : public Node {
public:
	Laplace() : Node("Laplace") {}

	void process() override;

	InPort<artemis::data::ScalarField> in_field{this, "in"};

	OutPort<artemis::data::ScalarField> out{this, "out"};
};

} // graph
