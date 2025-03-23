//
// Created by Michael Ankele on 2025-03-10.
//

#pragma once

#include <graph/Node.h>
#include <graph/Port.h>
#include <data/grid/RegularGrid.h>
#include <data/field/ScalarField.h>
#include <graph/Setting.h>
#include <lib/kaba/kaba.h>

namespace graph {

class ScalarField : public ResourceNode {
public:
	ScalarField();

	void process() override;

	Setting<string> formula{this, "formula", "0"};
	Setting<int> type{this, "type", 1};
	Setting<bool> time_dependent{this, "time-dependent", false};

	InPort<artemis::data::RegularGrid> in_grid{this, "grid"};

	OutPort<artemis::data::ScalarField> out_field{this, "out", PortFlags::Mutable};

	string cached_formula;
	owned<kaba::Context> ctx;
	shared<kaba::Module> module;
	typedef float (*f_t)(const vec3&, float);
	f_t f_p = nullptr;
};

} // graph

