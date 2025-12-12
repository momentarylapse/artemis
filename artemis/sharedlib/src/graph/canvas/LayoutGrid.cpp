//
// Created by michi on 8/26/25.
//

#include "LayoutGrid.h"
#include <view/Canvas.h>

namespace artemis::graph {

LayoutGrid::LayoutGrid(Session* s) : RendererNode(s, "LayoutGrid") {
	layout_grid = new view::LayoutGrid(s);
}

void LayoutGrid::on_process() {
	RenderData d;

	Array<view::RenderNode*> children;
	children.resize(4);
	if (in_a.has_value())
		children[0] = in_a.value()->render_node;
	if (in_b.has_value())
		children[1] = in_b.value()->render_node;
	if (in_c.has_value())
		children[2] = in_c.value()->render_node;
	if (in_d.has_value())
		children[3] = in_d.value()->render_node;
	layout_grid->set_children(children, horizontal());
	layout_grid->configure(spacing(), margin());
	d.render_node = layout_grid.get();
	out_draw(d);
}

}
