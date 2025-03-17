//
// Created by Michael Ankele on 2025-03-17.
//

#ifndef CANVAS_H
#define CANVAS_H

#include "RendererNode.h"

namespace graph {

class Canvas : public Node {
public:
	explicit Canvas(Session* s) : Node("Canvas") {
		flags = NodeFlags::Canvas;
	}

	InPort<RenderData> in_draw_3d{this, "draw-3d", PortFlags::Multi};
};

} // graph

#endif //CANVAS_H
