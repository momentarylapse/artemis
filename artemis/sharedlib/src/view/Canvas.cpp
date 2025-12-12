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

void LayoutGrid::configure(int spacing, int margin) {
	set_options("grid", format("spacing=%d,margin=%d", spacing, margin));
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

	LayoutNested::LayoutNested(Session *s) : RenderNode(s) {
	from_source(R"foodelim(
Dialog ? x
	Overlay ? ''
		Grid grid-main '' expandx expandy
		Grid grid-overlay '' spacing=40 margin=40
			.
			---|
			Label ? '' expandy
			---|
			Label ? '' expandy
)foodelim");
}

void LayoutNested::set_children(RenderNode* __main, const Array<RenderNode*>& _children, int mode) {
	if (_children == children and __main == this->_main)
		return;
	if (this->_main)
		unembed(this->_main);
	for (auto d: dummies)
		unembed(d);
	for (auto c: children)
		if (c)
			unembed(c);
	dummies.clear();
	this->_main = __main;
	children = _children;
	if (this->_main)
		embed("grid-main", 0, 0, this->_main);
	for (const auto& [i, c]: enumerate(children))
		if (c) {
			embed("grid-overlay", i, 0, c);
		} else {
			auto d = new xhui::Panel(format("dummy%d", i));
			d->size_mode_x = SizeMode::Expand;
			d->size_mode_y = SizeMode::Expand;
			embed("grid-overlay", i, 0, d);
			dummies.add(d);
		}
}

}
