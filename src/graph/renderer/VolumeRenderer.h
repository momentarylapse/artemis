//
// Created by Michael Ankele on 2025-03-10.
//

#ifndef VOLUMERENDERER_H
#define VOLUMERENDERER_H

#include <data/field/ScalarField.h>
#include <data/grid/RegularGrid.h>
#include <lib/dataflow/Port.h>

#include "RendererNode.h"

namespace artemis::graph {

class VolumeRenderer : public RendererNode {
public:
	explicit VolumeRenderer(Session* s);

	void process() override;

	void draw_win(const RenderParams& params, MultiViewWindow* win, RenderViewData& rvd) override;

	dataflow::InPort<data::ScalarField> in_field{this, "field"};

	owned<VertexBuffer> vertex_buffer;
};

} // graph

#endif //VOLUMERENDERER_H
