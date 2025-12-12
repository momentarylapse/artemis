//
// Created by Michael Ankele on 2025-03-18.
//

#ifndef LISTPLOT_H
#define LISTPLOT_H

#include "Plotter.h"
#include <lib/dataflow/Setting.h>
#include <lib/image/color.h>

namespace artemis::graph {

class ListPlot : public dataflow::Node {
public:
	explicit ListPlot() : Node("ListPlot") {}

	void on_process() override;

	dataflow::Setting<double> line_width{this, "line-width", 2.0};
	dataflow::Setting<color> _color{this, "color", Red};
	dataflow::Setting<double> x_scale{this, "x-scale", 1.0};

	dataflow::InPort<Array<double>> in_list{this, "list"};
	dataflow::OutPort<PlotData> out_plot{this, "plot"};
};

} // graph

#endif //LISTPLOT_H
