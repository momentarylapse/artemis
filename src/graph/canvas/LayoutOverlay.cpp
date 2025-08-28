//
// Created by michi on 8/28/25.
//

#include "LayoutOverlay.h"
#include <view/Canvas.h>

namespace artemis::graph {

LayoutOverlay::LayoutOverlay(Session* s) : RendererNode(s, "LayoutOverlay") {
	layout_overlay = new view::LayoutOverlay(s);
}

void LayoutOverlay::on_process() {
	RenderData d;

	Array<view::RenderNode*> children;
	children.resize(4);
	if (in_a.has_value())
		children[0] = in_a.value()->render_node;
	if (in_b.has_value())
		children[1] = in_b.value()->render_node;
	if (in_c.has_value())
		children[2] = in_c.value()->render_node;
	layout_overlay->set_children(children);
	d.render_node = layout_overlay.get();
	out_draw(d);
}

}