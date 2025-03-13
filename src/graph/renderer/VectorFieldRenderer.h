//
// Created by Michael Ankele on 2025-03-11.
//

#pragma once

#include "RendererNode.h"
#include <data/field/VectorField.h>
#include <graph/Port.h>
#include <lib/image/color.h>

namespace graph {

class VectorFieldRenderer : public RendererNode {
public:
	explicit VectorFieldRenderer(Session* s) : RendererNode(s, "VectorFieldRenderer") {}

	void draw_win(const RenderParams& params, MultiViewWindow* win) override;

	Setting<float> line_width{this, "line-width", 2.0f};
	Setting<color> _color{this, "color", Green};

	InPort<artemis::data::VectorField> in_field{this, "field"};
};

} // graph

