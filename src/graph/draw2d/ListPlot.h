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

	void process() override;

	dataflow::Setting<float> line_width{this, "line-width", 2.0f};
	dataflow::Setting<color> _color{this, "color", Red};

	dataflow::InPort<Array<float>> in_list{this, "list"};
	dataflow::OutPort<PlotData> out_plot{this, "plot"};
};

} // graph

#endif //LISTPLOT_H
