//
// Created by michi on 8/28/25.
//

#include "LayoutNested.h"
#include <view/Canvas.h>

namespace artemis::graph {

LayoutNested::LayoutNested(Session* s) : RendererNode(s, "LayoutNested") {
	layout_nested = new view::LayoutNested(s);
}

void LayoutNested::on_process() {
	RenderData d;

	view::RenderNode* _main = nullptr;
	Array<view::RenderNode*> children;
	children.resize(3);
	if (in_main.has_value())
		_main = in_main.value()->render_node;
	if (in_a.has_value())
		children[0] = in_a.value()->render_node;
	if (in_b.has_value())
		children[1] = in_b.value()->render_node;
	if (in_c.has_value())
		children[2] = in_c.value()->render_node;
	layout_nested->set_children(_main, children, mode());
	d.render_node = layout_nested.get();
	out_draw(d);
}

}