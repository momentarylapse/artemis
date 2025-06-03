//
// Created by Michael Ankele on 2025-03-10.
//

#ifndef VOLUMERENDERER_H
#define VOLUMERENDERER_H

#include <data/field/ScalarField.h>
#include <data/util/ColorMap.h>
#include <lib/dataflow/Port.h>
#include "RendererNode.h"

class Material;

namespace artemis::graph {

class VolumeRenderer : public RendererNode {
public:
	explicit VolumeRenderer(Session* s);

	void process() override;

	void draw_win(const RenderParams& params, MultiViewWindow* win, RenderViewData& rvd) override;

	dataflow::InPort<data::ScalarField> in_field{this, "field"};
	dataflow::Setting<data::ColorMap> color_map{this, "color_map", data::ColorMap::_default};

	owned<VertexBuffer> vertex_buffer;
	owned<VolumeTexture> tex;
	owned<Material> material;
};

} // graph

#endif //VOLUMERENDERER_H
