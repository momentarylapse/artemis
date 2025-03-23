//
// Created by Michael Ankele on 2025-03-10.
//

#ifndef POINTLISTRENDERER_H
#define POINTLISTRENDERER_H

#include "RendererNode.h"
#include <graph/Port.h>
#include <graph/Setting.h>
#include <data/mesh/PolygonMesh.h>
#include <lib/image/color.h>

class Material;

namespace graph {

class PointListRenderer : public RendererNode {
public:
	explicit PointListRenderer(Session* s);

	void process() override;

	void draw_win(const RenderParams& params, MultiViewWindow* win) override;

	Setting<float> radius{this, "radius", 0.2f};
	Setting<color> _color{this, "color", White};

	InPort<Array<vec3>> in_points{this, "points"};
	InPort<PolygonMesh> in_mesh{this, "mesh", PortFlags::Optional};

	owned<VertexBuffer> vertex_buffer;
	owned<Material> material;
};

} // graph

#endif //POINTLISTRENDERER_H
