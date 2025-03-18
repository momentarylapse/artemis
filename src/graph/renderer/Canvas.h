//
// Created by Michael Ankele on 2025-03-17.
//

#ifndef CANVAS_H
#define CANVAS_H

#include "RendererNode.h"

class rect;

namespace graph {

class Canvas : public Node {
public:
	explicit Canvas(Session* s) : Node("Canvas") {
		flags = NodeFlags::Canvas;
	}

	void draw_win(const RenderParams& params, MultiViewWindow* win);
	void draw_2d(Painter* p);

	InPort<RenderData> in_draw{this, "draw", PortFlags::Multi};
};

} // graph

#endif //CANVAS_H
