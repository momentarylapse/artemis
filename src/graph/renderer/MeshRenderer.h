//
// Created by michi on 09.03.25.
//

#ifndef MESHRENDERER_H
#define MESHRENDERER_H

#include "RendererNode.h"
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>
#include <data/mesh/PolygonMesh.h>
#include <y/graphics-fwd.h>

namespace artemis::graph {

class MeshRenderer : public RendererNode {
public:
	explicit MeshRenderer(Session* s);
	~MeshRenderer() override;

	void on_process() override;

	void draw_win(const RenderParams& params, MultiViewWindow* win, RenderViewData& rvd);

	dataflow::InPort<PolygonMesh> in_mesh{this, "mesh"};

	dataflow::Setting<double> roughness{this, "roughness", 0.7};
	dataflow::Setting<double> metal{this, "metal", 0.05};
	dataflow::Setting<color> albedo{this, "albedo", White};
	dataflow::Setting<color> emission{this, "emission", Black};

	owned<VertexBuffer> vertex_buffer;
	owned<Material> material;
};

} // graph

#endif //MESHRENDERER_H
