
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

	void on_process() override;

	void draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win, yrenderer::RenderViewData& rvd);

	dataflow::Setting<double> scale{this, "scale", 1.0};
	dataflow::Setting<double> radius{this, "radius", 0.1};

	dataflow::InPort<artemis::data::VectorField> in_diff{this, "diff"};
	dataflow::InPort<artemis::data::ScalarField> in_scalar{this, "scalar", dataflow::PortFlags::Optional};

	owned<ygfx::VertexBuffer> vertex_buffer;
};

} // graph
