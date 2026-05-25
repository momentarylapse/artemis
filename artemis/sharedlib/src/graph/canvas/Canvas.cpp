//
// Created by Michael Ankele on 2025-03-17.
//

#include "Canvas.h"
#include "view/Canvas.h"
#include <Session.h>
#include <lib/profiler/Profiler.h>
#include "view/ArtemisWindow.h"

namespace artemis {
	extern Session* default_session;
}

namespace artemis::graph {

RendererNode::RendererNode(const string& name) : Node(name) {
	session = default_session;
	flags = dataflow::NodeFlags::Renderer;
	channel_draw = profiler::create_channel(name + ":draw", channel);
}

void Canvas::on_process() {
	view::RenderNode* child = nullptr;
	if (in_draw.has_value())
		child = in_draw.value()->render_node;
	session->win->canvas->set_child(child);
}



} // graph