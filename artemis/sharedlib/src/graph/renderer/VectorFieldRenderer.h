//
// Created by Michael Ankele on 2025-03-11.
//

#pragma once

#include "RendererNode.h"
#include <data/field/VectorField.h>
#include <lib/dataflow/Port.h>
#include <lib/image/color.h>

namespace artemis::graph {

class VectorFieldRenderer : public RenderEmitterNode {
public:
	explicit VectorFieldRenderer(Session* s) : RenderEmitterNode(s, "VectorFieldRenderer") {}

	void on_process() override;

	void on_emit(const yrenderer::RenderParams &params, yrenderer::RenderViewData &rvd, bool shadow_pass) override;
	base::optional<Box> bounding_box() const override;

	dataflow::Setting<double> line_width{this, "line-width", 2.0, "range=0:99:0.1"};
	dataflow::Setting<double> scale{this, "scale", 1.0};
	dataflow::Setting<color> _color{this, "color", Green};

	dataflow::InPort<data::VectorField> in_field{this, "field"};
};

} // graph

