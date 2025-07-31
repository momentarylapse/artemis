//
// Created by Michael Ankele on 2025-03-17.
//

#ifndef CANVAS_H
#define CANVAS_H

#include "RendererNode.h"
#include <lib/image/color.h>

class rect;

namespace artemis::graph {

class Canvas : public dataflow::Node {
public:
	explicit Canvas(Session* s) : Node("Canvas") {
		session = s;
		flags = dataflow::NodeFlags::Canvas;
	}

	void draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win, yrenderer::RenderViewData& rvd);
	void draw_2d(Painter* p);

	dataflow::Setting<color> background{this, "background", color(-1,-1,-1,-1)};

	dataflow::InPort<RenderData> in_draw{this, "draw", dataflow::PortFlags::Multi};

	Session* session;
	bool camera_defined = false;
};

} // graph

#endif //CANVAS_H
