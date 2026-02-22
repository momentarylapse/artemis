//
// Created by michi on 02.22.26.
//

#pragma once


#include "RendererNode.h"
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>
#include <lib/mesh/PolygonMesh.h>
#include <lib/ygraphics/graphics-fwd.h>
#include <lib/image/image.h>

namespace artemis::graph {

class ImageRenderer : public RenderEmitterNode {
public:
	explicit ImageRenderer(Session* s);
	~ImageRenderer() override;

	void on_process() override;

	void on_emit(const yrenderer::RenderParams &params, yrenderer::RenderViewData &rvd, bool shadow_pass) override;
	base::optional<Box> bounding_box() const override;

	dataflow::InPort<Image> in_image{this, "image"};

	owned<ygfx::VertexBuffer> vertex_buffer;
	owned<yrenderer::Material> material;
};

} // graph
