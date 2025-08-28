//
// Created by michi on 8/26/25.
//

#include "Canvas.h"
#include <lib/base/iter.h>
#include <lib/xhui/Painter.h>
#include "ArtemisWindow.h"
#include "SceneRenderer.h"
#include "Session.h"
#include "lib/os/msg.h"

namespace artemis::view {

RenderNode::RenderNode(Session* _session) : xhui::Panel(p2s(this)) {
	session = _session;
}

Canvas::Canvas(Session *s) : RenderNode(s) {
	add_control("Grid", "", 0, 0, "grid"); // not sure we even need this...
}

void Canvas::set_child(RenderNode* c) {
	if (c == child)
		return;
	if (child)
		unembed(child);
	child = c;
	if (child)
		embed("grid", 0, 0, child);
}



LayoutGrid::LayoutGrid(Session *s) : RenderNode(s) {
	add_control("Grid", "", 0, 0, "grid");
	set_options("grid", "spacing=0");
}

void LayoutGrid::set_children(const Array<RenderNode*>& _children, bool _horizontal) {
	if (_children == children and _horizontal == horizontal)
		return;
	for (auto c: children)
		if (c)
			unembed(c);
	children = _children;
	horizontal = _horizontal;
	for (const auto& [i, c]: enumerate(children))
		if (c)
			embed("grid", horizontal ? i : 0, horizontal ? 0 : i, c);
}

void LayoutGrid::configure(int spacing, int padding) {
	set_options("grid", format("spacing=%d,padding=%d", spacing, padding));
}

LayoutOverlay::LayoutOverlay(Session *s) : RenderNode(s) {
	add_control("Overlay", "", 0, 0, "overlay");
}

void LayoutOverlay::set_children(const Array<RenderNode*>& _children) {
	if (_children == children)
		return;
	for (auto c: children)
		if (c)
			unembed(c);
	children = _children;
	for (const auto& [i, c]: enumerate(children))
		if (c)
			embed("overlay", i, 0, c);
}

}
