//
// Created by Michael Ankele on 2025-03-10.
//

#ifndef POINTLISTRENDERER_H
#define POINTLISTRENDERER_H

#include "RendererNode.h"
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>
#include <data/mesh/PolygonMesh.h>
#include <lib/image/color.h>

class Material;

namespace artemis::graph {

class PointListRenderer : public RendererNode {
public:
	explicit PointListRenderer(Session* s);

	void process() override;

	void draw_win(const RenderParams& params, MultiViewWindow* win, RenderViewData& rvd) override;

	dataflow::Setting<float> radius{this, "radius", 0.2f, "range=0:99:0.1"};
	dataflow::Setting<color> _color{this, "color", White};

	dataflow::InPort<Array<vec3>> in_points{this, "points"};
	dataflow::InPort<PolygonMesh> in_mesh{this, "mesh", dataflow::PortFlags::Optional};

	owned<VertexBuffer> vertex_buffer;
	owned<Material> material;
};

} // graph

#endif //POINTLISTRENDERER_H
