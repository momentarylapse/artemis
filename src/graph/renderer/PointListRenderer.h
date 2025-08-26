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

namespace yrenderer {
	class Material;
}

namespace artemis::graph {

class PointListRenderer : public RenderEmitterNode {
public:
	explicit PointListRenderer(Session* s);

	void on_process() override;

	base::optional<Box> bounding_box() const override;
	void on_emit(const yrenderer::RenderParams &params, yrenderer::RenderViewData &rvd, bool shadow_pass) override;

	dataflow::Setting<double> radius{this, "radius", 0.2, "range=0:99:0.1"};
	dataflow::Setting<int> trail_length{this, "trail-length", 0, "range=0:"};
	dataflow::Setting<double> line_width{this, "line-width", 1, "range=0:99:0.1"};
	dataflow::Setting<color> _color{this, "color", White};

	dataflow::InPort<Array<vec3>> in_points{this, "points"};
	dataflow::InPort<PolygonMesh> in_mesh{this, "mesh", dataflow::PortFlags::Optional};

	owned<ygfx::VertexBuffer> vertex_buffer;
	owned<yrenderer::Material> material;

	Array<Array<vec3>> trails;
};

} // graph

#endif //POINTLISTRENDERER_H
