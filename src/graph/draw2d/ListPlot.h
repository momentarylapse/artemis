//
// Created by Michael Ankele on 2025-03-18.
//

#ifndef LISTPLOT_H
#define LISTPLOT_H

#include "Plotter.h"
#include <graph/Setting.h>
#include <lib/image/color.h>

namespace graph {

class ListPlot : public Node {
public:
	explicit ListPlot() : Node("ListPlot") {}

	void process() override;

	Setting<float> line_width{this, "line-width", 2.0f};
	Setting<color> _color{this, "color", Red};

	InPort<Array<float>> in_list{this, "list"};
	OutPort<PlotData> out_plot{this, "plot"};
};

} // graph

#endif //LISTPLOT_H
