//
// Created by Michael Ankele on 2025-03-16.
//

#pragma once

#include "../renderer/RendererNode.h"
#include <graph/Setting.h>
#include <lib/image/color.h>
#include <lib/kaba/kaba.h>

namespace graph {

typedef float (*scalar_function_1d)(float);

struct PlotData {
	float line_width;
	color _color;
	scalar_function_1d f;
	Array<vec2> points;
};

class Plotter : public RendererNode {
public:
	explicit Plotter(Session* s) : RendererNode(s, "Plotter") {}

	void process() override;

	void draw_win(const RenderParams& params, MultiViewWindow* win) override {}
	void draw_2d(Painter* p);


	Setting<bool> auto_zoom{this, "auto-zoom", true};
	Setting<float> x_min{this, "x-min", -10};
	Setting<float> x_max{this, "x-max", 10};

	InPort<PlotData> in_plot{this, "plot", PortFlags::Multi};
};

} // graph

