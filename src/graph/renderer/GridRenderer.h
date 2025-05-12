//
// Created by Michael Ankele on 2025-03-10.
//

#ifndef GRIDRENDERER_H
#define GRIDRENDERER_H

#include "RendererNode.h"
#include <data/grid/RegularGrid.h>
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>
#include <lib/image/color.h>

namespace artemis::graph {

class GridRenderer : public RendererNode {
public:
	explicit GridRenderer(Session* s) : RendererNode(s, "GridRenderer") {}

	void process() override;

	void draw_win(const RenderParams& params, MultiViewWindow* win, RenderViewData& rvd) override;

	dataflow::Setting<float> line_width{this, "line-width", 1.0f, "range=0:99:0.1"};
	dataflow::Setting<color> _color{this, "color", Gray};

	dataflow::InPort<data::RegularGrid> regular{this, "regular"};
};

} // graph

#endif //GRIDRENDERER_H
