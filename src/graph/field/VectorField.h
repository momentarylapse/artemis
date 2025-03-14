//
// Created by Michael Ankele on 2025-03-14.
//

#pragma once

#include <graph/Node.h>
#include <graph/Port.h>
#include <data/grid/RegularGrid.h>
#include <data/field/VectorField.h>
#include <graph/Setting.h>
#include <lib/kaba/kaba.h>

namespace graph {

class VectorField : public ResourceNode {
public:
	VectorField() : ResourceNode("VectorField") {}

	void process() override;

	Setting<string> formula{this, "formula", "[0,0,0]"};
	Setting<bool> time_dependent{this, "time-dependent", false};

	InPort<artemis::data::RegularGrid> in_grid{this, "grid"};

	OutPort<artemis::data::VectorField> out_field{this, "out", PortFlags::Mutable};

	string cached_formula;
	owned<kaba::Context> ctx;
	shared<kaba::Module> module;
	typedef vec3 (*f_t)(const vec3&, float);
	f_t f_p = nullptr;
};

} // graph

