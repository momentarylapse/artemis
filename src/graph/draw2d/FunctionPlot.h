//
// Created by Michael Ankele on 2025-03-17.
//

#ifndef FUNCTIONPLOT_H
#define FUNCTIONPLOT_H

#include "Plotter.h"
#include <lib/dataflow/Setting.h>
#include <lib/image/color.h>
#include <lib/kaba/kaba.h>

namespace artemis::graph {

class FunctionPlot : public dataflow::Node {
public:
	explicit FunctionPlot() : Node("FunctionPlot") {}

	void on_process() override;

	dataflow::Setting<string> formula{this, "formula", "x^2"};
	dataflow::Setting<double> line_width{this, "line-width", 2.0};
	dataflow::Setting<color> _color{this, "color", Red};

	dataflow::OutPort<PlotData> out_plot{this, "plot"};

	string cached_formula;
	owned<kaba::Context> ctx;
	shared<kaba::Module> module;
	typedef float (*f_t)(float);
	f_t f_p = nullptr;

};

} // graph

#endif //FUNCTIONPLOT_H
