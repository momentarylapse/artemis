
#pragma once

#include "RendererNode.h"
#include <data/field/ScalarField.h>
#include <data/field/VectorField.h>
#include <graph/Port.h>
#include <graph/Setting.h>

namespace graph {

class DeformationRenderer : public RendererNode {
public:
	explicit DeformationRenderer(Session* s);

	void draw_win(const RenderParams& params, MultiViewWindow* win) override;

	Setting<float> scale{this, "scale", 1.0f};
	Setting<float> radius{this, "radius", 0.1f};

	InPort<artemis::data::VectorField> in_diff{this, "diff"};
	InPort<artemis::data::ScalarField> in_scalar{this, "scalar", PortFlags::Optional};

	owned<VertexBuffer> vertex_buffer;
};

} // graph
