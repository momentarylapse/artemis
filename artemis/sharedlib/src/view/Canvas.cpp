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

namespace artemis {
	extern Session* default_session;
}

namespace artemis::view {

RenderNode::RenderNode() : xhui::Panel(p2s(this)) {
	session = default_session;
}

Canvas::Canvas() : RenderNode() {
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

}
