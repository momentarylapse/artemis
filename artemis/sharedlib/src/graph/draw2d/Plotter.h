//
// Created by Michael Ankele on 2025-03-16.
//

#pragma once

#include "../canvas/Canvas.h"
#include <lib/dataflow/Setting.h>
#include <lib/image/color.h>
#include <lib/base/pointer.h>

namespace artemis::view {
	class RenderNode;
}

namespace artemis::graph {

class PlotSource;

typedef float (*scalar_function_1d)(float); // double..?

enum class PlotMode {
	Function,
	Points
};

struct PlotData {
	PlotSource* source = nullptr;
	PlotMode mode;
	int dummy[8];
};

class PlotSource : public dataflow::Node {
public:
	explicit PlotSource(const string& name) : Node(name) {}

	virtual float plot_function(float x) { return 0; }
	virtual Array<vec2> plot_points() { return {}; }

	dataflow::Setting<double> line_width{this, "line-width", 2.0};
	dataflow::Setting<color> _color{this, "color", Red};
	dataflow::OutPort<PlotData> out_plot{this, "plot"};
};

class Plotter : public RendererNode {
public:
	explicit Plotter();

	void on_process() override;

	void draw_2d(Painter* p);


	dataflow::Setting<bool> auto_zoom{this, "auto-zoom", true};
	dataflow::Setting<double> x_min{this, "x-min", -10};
	dataflow::Setting<double> x_max{this, "x-max", 10};
	dataflow::Setting<double> y_min{this, "y-min", -5};
	dataflow::Setting<double> y_max{this, "y-max", 15};
	dataflow::Setting<color> background{this, "background", color(-1,-1,-1,-1)};

	dataflow::InPort<PlotData> in_plot{this, "plot", dataflow::PortFlags::Multi};

	shared<view::RenderNode> render_node;
};

} // graph

