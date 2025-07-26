//
// Created by Michael Ankele on 2025-03-16.
//

#pragma once

#include "../renderer/RendererNode.h"
#include <lib/dataflow/Setting.h>
#include <lib/image/color.h>

namespace artemis::graph {

typedef float (*scalar_function_1d)(float); // double..?

struct PlotData {
	double line_width;
	color _color;
	scalar_function_1d f;
	Array<vec2> points;
};

class Plotter : public RendererNode {
public:
	explicit Plotter(Session* s) : RendererNode(s, "Plotter") {}

	void on_process() override;

	void draw_2d(Painter* p);


	dataflow::Setting<bool> auto_zoom{this, "auto-zoom", true};
	dataflow::Setting<double> x_min{this, "x-min", -10};
	dataflow::Setting<double> x_max{this, "x-max", 10};

	dataflow::InPort<PlotData> in_plot{this, "plot", dataflow::PortFlags::Multi};
};

} // graph

