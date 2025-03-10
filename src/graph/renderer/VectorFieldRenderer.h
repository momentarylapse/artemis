//
// Created by Michael Ankele on 2025-03-11.
//

#pragma once

#include "RendererNode.h"
#include <data/field/VectorField.h>
#include <graph/Port.h>

namespace graph {

class VectorFieldRenderer : public RendererNode {
public:
	explicit VectorFieldRenderer(Session* s) : RendererNode(s, "VectorFieldRenderer") {}

	void draw_win(const RenderParams& params, MultiViewWindow* win) override;

	InPort<artemis::data::VectorField> in_field{this, "field"};
};

} // graph

