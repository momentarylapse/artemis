//
// Created by michi on 09.03.25.
//

#include "ModeDefault.h"

#include <Session.h>
#include <graph/Graph.h>
#include <graph/Node.h>
#include <graph/Port.h>
#include <graph/renderer/Canvas.h>
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
		if (simulation_active)
			graph->iterate_simulation(0.1f);
		if (graph->iterate())
			session->win->request_redraw();
	});

	auto win = session->win;
	win->event("simulation-start", [this] {
		simulation_active = true;
		update_menu();
	});
	win->event("simulation-pause", [this] {
		simulation_active = !simulation_active;
		update_menu();
	});
	win->event("simulation-stop", [this] {
		simulation_active = false;
		_current_simulation_time_ = 0;
		graph->reset_state();
		update_menu();
	});

	update_menu();
}

void ModeDefault::update_menu() {
	auto win = session->win;
	win->enable("simulation-start", !simulation_active);
	win->enable("simulation-pause", simulation_active or _current_simulation_time_ > 0);
	win->enable("simulation-stop", simulation_active or _current_simulation_time_ > 0);
}


void ModeDefault::on_draw_win(const RenderParams& params, MultiViewWindow* win) {
	session->drawing_helper->clear(params, xhui::Theme::_default.background_low);

	for (auto n: graph->nodes)
		if (n->flags & graph::NodeFlags::Renderer) {
			auto r = static_cast<graph::RendererNode*>(n);
			if (r->active())
				r->draw_win(params, win);
		}
}

void ModeDefault::on_draw_post(Painter* p) {

	for (auto n: graph->nodes)
		if (n->flags & graph::NodeFlags::Canvas) {
			auto c = static_cast<graph::Canvas*>(n);
			c->draw_2d(p);
		}

	p->set_color(White);
	p->draw_str(p->area().p11() - vec2(100, 50), format("t = %.1f", _current_simulation_time_));
}

void ModeDefault::on_key_down(int key) {
	if (key == xhui::KEY_S + xhui::KEY_CONTROL) {

	}
}




