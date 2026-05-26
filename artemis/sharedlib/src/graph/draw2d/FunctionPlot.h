//
// Created by Michael Ankele on 2025-03-17.
//

#pragma once

#include "Plotter.h"
#include <lib/dataflow/Setting.h>
#include <lib/image/color.h>
#include <lib/kaba/kaba.h>

namespace artemis::graph {

class FunctionPlot : public PlotSource {
public:
	explicit FunctionPlot() : PlotSource("FunctionPlot") {}

	void on_process() override;
	float plot_function(float x) override;

	dataflow::Setting<string> formula{this, "formula", "x^2"};

	string cached_formula;
	owned<kaba::IContext> ctx;
	shared<kaba::Module> module;
	typedef float (*f_t)(float);
	f_t f_p = nullptr;

};

} // graph
