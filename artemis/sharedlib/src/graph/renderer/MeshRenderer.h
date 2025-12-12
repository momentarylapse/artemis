//
// Created by michi on 09.03.25.
//

#pragma once


#include "RendererNode.h"
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>
#include <lib/mesh/PolygonMesh.h>
#include <lib/ygraphics/graphics-fwd.h>

namespace artemis::graph {

class MeshRenderer : public RenderEmitterNode {
public:
	explicit MeshRenderer(Session* s);
	~MeshRenderer() override;

	void on_process() override;

	void on_emit(const yrenderer::RenderParams &params, yrenderer::RenderViewData &rvd, bool shadow_pass) override;
	base::optional<Box> bounding_box() const override;

	dataflow::InPort<PolygonMesh> in_mesh{this, "mesh"};

	dataflow::Setting<double> roughness{this, "roughness", 0.7};
	dataflow::Setting<double> metal{this, "metal", 0.05};
	dataflow::Setting<color> albedo{this, "albedo", White};
	dataflow::Setting<color> emission{this, "emission", Black};

	owned<ygfx::VertexBuffer> vertex_buffer;
	owned<yrenderer::Material> material;
};

} // graph
