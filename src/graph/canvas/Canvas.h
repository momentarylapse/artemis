//
// Created by Michael Ankele on 2025-03-17.
//

#pragma once

//#include "RendererNode.h"
#include <lib/base/pointer.h>
#include <lib/base/optional.h>
#include <lib/dataflow/Node.h>
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>
#include <lib/math/Box.h>
#include <lib/image/color.h>

class rect;

namespace yrenderer {
	struct RenderViewData;
	struct RenderParams;
	class MeshEmitter;
}
class Session;
class Painter;

namespace artemis::view {
	class RenderNode;
}

namespace artemis::graph {


struct RenderData {
	// TODO
	bool active;
	base::optional<Box> bounding_box;
	view::RenderNode* render_node = nullptr;
};

class RendererNode : public dataflow::Node {
public:
	explicit RendererNode(Session* s, const string& name);

	dataflow::OutPort<RenderData> out_draw{this, "draw"};

	Session* session;
};

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
