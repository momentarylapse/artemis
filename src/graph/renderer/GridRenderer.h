//
// Created by Michael Ankele on 2025-03-10.
//

#ifndef GRIDRENDERER_H
#define GRIDRENDERER_H

#include "RendererNode.h"
#include <data/grid/RegularGrid.h>
#include <graph/Port.h>
#include <graph/Setting.h>
#include <lib/image/color.h>

namespace graph {

class GridRenderer : public RendererNode {
public:
	explicit GridRenderer(Session* s) : RendererNode(s, "GridRenderer") {}

	void process() override;

	void draw_win(const RenderParams& params, MultiViewWindow* win) override;

	Setting<float> line_width{this, "line-width", 1.0f, "range=0:99:0.1"};
	Setting<color> _color{this, "color", Gray};

	InPort<artemis::data::RegularGrid> regular{this, "regular"};
};

} // graph

#endif //GRIDRENDERER_H
