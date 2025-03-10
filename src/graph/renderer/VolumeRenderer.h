//
// Created by Michael Ankele on 2025-03-10.
//

#ifndef VOLUMERENDERER_H
#define VOLUMERENDERER_H

#include <data/field/ScalarField.h>
#include <data/grid/RegularGrid.h>
#include <graph/Port.h>

#include "RendererNode.h"

namespace graph {

class VolumeRenderer : public RendererNode {
public:
	explicit VolumeRenderer(Session* s);

	void draw_win(const RenderParams& params, MultiViewWindow* win) override;

	InPort<artemis::data::ScalarField> in_field{this, "field"};

	owned<VertexBuffer> vertex_buffer;
};

} // graph

#endif //VOLUMERENDERER_H
