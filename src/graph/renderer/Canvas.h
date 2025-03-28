//
// Created by Michael Ankele on 2025-03-17.
//

#ifndef CANVAS_H
#define CANVAS_H

#include "RendererNode.h"
#include <lib/image/color.h>

class rect;

namespace graph {

class Canvas : public Node {
public:
	explicit Canvas(Session* s) : Node("Canvas") {
		session = s;
		flags = NodeFlags::Canvas;
	}

	void draw_win(const RenderParams& params, MultiViewWindow* win);
	void draw_2d(Painter* p);

	Setting<color> background{this, "background", color(-1,-1,-1,-1)};

	InPort<RenderData> in_draw{this, "draw", PortFlags::Multi};

	Session* session;
	bool camera_defined = false;
};

} // graph

#endif //CANVAS_H
