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

	void draw_win(const RenderParams& params, MultiViewWindow* win, RenderViewData& rvd);

	dataflow::Setting<float> line_width{this, "line-width", 2.0f, "range=0:99:0.1"};
	dataflow::Setting<color> _color{this, "color", Green};

	dataflow::InPort<data::VectorField> in_field{this, "field"};
};

} // graph

