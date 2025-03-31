//
// Created by Michael Ankele on 2025-03-10.
//

#pragma once

#include <lib/dataflow/ResourceNode.h>
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>
#include <data/grid/RegularGrid.h>
#include <data/field/ScalarField.h>
#include <lib/kaba/kaba.h>

namespace artemis::graph {

class ScalarField : public dataflow::ResourceNode<data::ScalarField> {
public:
	ScalarField();

	void process() override;

	dataflow::Setting<string> formula{this, "formula", "0"};
	dataflow::SettingFromSet<data::ScalarType> type{this, "type", data::ScalarType::Float32,
		{data::ScalarType::Float32, artemis::data::ScalarType::Float64}, {"f32", "f64"}};
	dataflow::SettingFromSet<data::SamplingMode> sampling_mode{this, "sampling-mode", data::SamplingMode::PerCell,
		{data::SamplingMode::PerCell, data::SamplingMode::PerVertex}, {"PerCell", "PerVertex"}};
	dataflow::Setting<bool> time_dependent{this, "time-dependent", false};

	dataflow::InPort<data::RegularGrid> in_grid{this, "grid"};

	string cached_formula;
	owned<kaba::Context> ctx;
	shared<kaba::Module> module;
	typedef float (*f_t)(const vec3&, float);
	f_t f_p = nullptr;
};

} // graph

