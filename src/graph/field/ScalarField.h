//
// Created by Michael Ankele on 2025-03-10.
//

#pragma once

#include <lib/dataflow/Node.h>
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>
#include <data/grid/RegularGrid.h>
#include <data/field/ScalarField.h>
#include <lib/kaba/kaba.h>

namespace artemis::graph {

class ScalarField : public dataflow::ResourceNode {
public:
	ScalarField();

	void process() override;

	dataflow::Setting<string> formula{this, "formula", "0"};
	dataflow::SettingFromSet<artemis::data::ScalarType> type{this, "type", artemis::data::ScalarType::Float32,
		{artemis::data::ScalarType::Float32, artemis::data::ScalarType::Float64}, {"f32", "f64"}};
	dataflow::SettingFromSet<artemis::data::SamplingMode> sampling_mode{this, "sampling-mode", artemis::data::SamplingMode::PerCell,
		{artemis::data::SamplingMode::PerCell, artemis::data::SamplingMode::PerVertex}, {"PerCell", "PerVertex"}};
	dataflow::Setting<bool> time_dependent{this, "time-dependent", false};

	dataflow::InPort<artemis::data::RegularGrid> in_grid{this, "grid"};

	dataflow::OutPort<artemis::data::ScalarField> out_field{this, "out", dataflow::PortFlags::Mutable};

	string cached_formula;
	owned<kaba::Context> ctx;
	shared<kaba::Module> module;
	typedef float (*f_t)(const vec3&, float);
	f_t f_p = nullptr;
};

} // graph

