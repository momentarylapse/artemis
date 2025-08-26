
#pragma once

#include "RendererNode.h"
#include <data/field/ScalarField.h>
#include <data/field/VectorField.h>
#include <lib/base/pointer.h>
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>
#include <lib/image/color.h>
#include <lib/ygraphics/graphics-fwd.h>

namespace artemis::graph {

class DeformationRenderer : public RenderEmitterNode {
public:
	explicit DeformationRenderer(Session* s);

	void on_process() override;

	base::optional<Box> bounding_box() const override;
	void on_emit(const yrenderer::RenderParams &params, yrenderer::RenderViewData &rvd, bool shadow_pass) override;

	dataflow::Setting<double> scale{this, "scale", 1.0};
	dataflow::Setting<double> radius{this, "radius", 0.1};
	dataflow::Setting<bool> show_balls{this, "show-balls", false};
	dataflow::Setting<bool> show_grid{this, "show-grid", true};
	dataflow::Setting<double> line_width{this, "line-width", 1.0, "range=0:99:0.1"};
	dataflow::Setting<color> _color{this, "color", Gray};

	dataflow::InPort<artemis::data::VectorField> in_diff{this, "diff"};
	dataflow::InPort<artemis::data::ScalarField> in_scalar{this, "scalar", dataflow::PortFlags::Optional};

	owned<ygfx::VertexBuffer> vertex_buffer;
};

} // graph
