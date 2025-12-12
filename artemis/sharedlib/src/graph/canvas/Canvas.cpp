//
// Created by Michael Ankele on 2025-03-17.
//

#include "Canvas.h"
#include "view/Canvas.h"

#include <Session.h>
#include <view/DrawingHelper.h>
#include "view/ArtemisWindow.h"

namespace artemis::graph {

RendererNode::RendererNode(Session* s, const string& name) : Node(name) {
	session = s;
	flags = dataflow::NodeFlags::Renderer;
}

void Canvas::on_process() {
	view::RenderNode* child = nullptr;
	if (in_draw.has_value())
		child = in_draw.value()->render_node;
	session->win->canvas->set_child(child);
}



} // graph