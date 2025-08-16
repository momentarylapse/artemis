//
// Created by Michael Ankele on 2025-03-11.
//

#pragma once

#include "RendererNode.h"
#include <data/field/VectorField.h>
#include <lib/dataflow/Port.h>
#include <lib/image/color.h>

namespace artemis::graph {

class VectorFieldRenderer : public RendererNode {
public:
	explicit VectorFieldRenderer(Session* s) : RendererNode(s, "VectorFieldRenderer") {}

	void on_process() override;

	void draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win, yrenderer::RenderViewData& rvd);

	dataflow::Setting<double> line_width{this, "line-width", 2.0, "range=0:99:0.1"};
	dataflow::Setting<double> scale{this, "scale", 1.0};
	dataflow::Setting<color> _color{this, "color", Green};

	dataflow::InPort<data::VectorField> in_field{this, "field"};
};

} // graph

