//
// Created by Michael Ankele on 2025-03-10.
//

#pragma once

#include <graph/Node.h>
#include <graph/Port.h>
#include <data/grid/RegularGrid.h>
#include <data/field/ScalarField.h>

namespace graph {

class ScalarField : public ResourceNode {
public:
	ScalarField() : ResourceNode("ScalarField") {}

	void process() override;

	InPort<artemis::data::RegularGrid> in_grid{this, "grid"};

	OutPort<artemis::data::ScalarField> out_field{this, "out"};
};

} // graph

