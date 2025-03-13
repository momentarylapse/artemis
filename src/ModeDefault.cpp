//
// Created by michi on 09.03.25.
//

#include "ModeDefault.h"

#include <Session.h>
#include <graph/Graph.h>
#include <graph/Node.h>
#include <graph/Port.h>
#include <graph/renderer/RendererNode.h>
#include <lib/xhui/dialogs/FileSelectionDialog.h>
#include <lib/os/msg.h>
#include <lib/xhui/Theme.h>
#include <view/ArtemisWindow.h>
#include <view/DrawingHelper.h>
#include <view/MultiView.h>

extern float _current_simulation_time_;

ModeDefault::ModeDefault(Session* s) : Mode(s) {
	multi_view = new MultiView(session);

	graph = session->graph.get();

	xhui::run_repeated(0.1f, [this] {
		_current_simulation_time_ += 0.1f;
		graph->iterate();
		session->win->request_redraw();
	});
}

void ModeDefault::on_draw_win(const RenderParams& params, MultiViewWindow* win) {
	session->drawing_helper->clear(params, xhui::Theme::_default.background_low);

	for (auto n: graph->nodes)
		if (n->is_renderer) {
			auto r = static_cast<graph::RendererNode*>(n);
			if (r->active())
				r->draw_win(params, win);
		}
}

void ModeDefault::on_draw_post(Painter* p) {
}

void ModeDefault::on_key_down(int key) {
	if (key == xhui::KEY_S + xhui::KEY_CONTROL) {

	}
}




