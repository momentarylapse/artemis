//
// Created by Michael Ankele on 2025-03-10.
//

#ifndef VOLUMERENDERER_H
#define VOLUMERENDERER_H

#include <data/field/ScalarField.h>
#include <data/util/ColorMap.h>
#include <lib/dataflow/Port.h>
#include "RendererNode.h"

namespace yrenderer {
	class Material;
}

namespace artemis::graph {

class VolumeRenderer : public RendererNode {
public:
	explicit VolumeRenderer(Session* s);

	void on_process() override;

	void draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win, yrenderer::RenderViewData& rvd);

	dataflow::InPort<data::ScalarField> in_field{this, "field"};
	dataflow::Setting<bool> solid{this, "solid", false};
	dataflow::Setting<double> scale{this, "scale", 1.0};
	dataflow::Setting<data::ColorMap> color_map{this, "color_map", data::ColorMap::_default_transparent};

	owned<ygfx::VertexBuffer> vertex_buffer;
	owned<ygfx::VolumeTexture> tex;
	owned<yrenderer::Material> material;
	owned<yrenderer::Material> material_solid;
};

} // graph

#endif //VOLUMERENDERER_H
