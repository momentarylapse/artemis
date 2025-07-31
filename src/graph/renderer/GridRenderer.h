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

	void on_process() override;

	void draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win, yrenderer::RenderViewData& rvd);

	dataflow::Setting<double> line_width{this, "line-width", 1.0, "range=0:99:0.1"};
	dataflow::Setting<color> _color{this, "color", Gray};

	dataflow::InPort<data::RegularGrid> regular{this, "regular"};
};

} // graph

#endif //GRIDRENDERER_H
