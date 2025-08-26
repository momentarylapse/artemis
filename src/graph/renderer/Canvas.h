//
// Created by Michael Ankele on 2025-03-17.
//

#pragma once

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

	void on_process() override;

	dataflow::InPort<RenderData> in_draw{this, "draw", dataflow::PortFlags::Optional};

	Session* session;
	bool camera_defined = false;
};

}
