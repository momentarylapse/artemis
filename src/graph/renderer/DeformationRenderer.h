
#pragma once

#include "RendererNode.h"
#include <data/field/ScalarField.h>
#include <data/field/VectorField.h>
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>

namespace artemis::graph {

class DeformationRenderer : public RendererNode {
public:
	explicit DeformationRenderer(Session* s);

	void process() override;

	void draw_win(const RenderParams& params, MultiViewWindow* win, RenderViewData& rvd) override;

	dataflow::Setting<float> scale{this, "scale", 1.0f};
	dataflow::Setting<float> radius{this, "radius", 0.1f};

	dataflow::InPort<artemis::data::VectorField> in_diff{this, "diff"};
	dataflow::InPort<artemis::data::ScalarField> in_scalar{this, "scalar", dataflow::PortFlags::Optional};

	owned<VertexBuffer> vertex_buffer;
};

} // graph
