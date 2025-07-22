//
// Created by michi on 09.03.25.
//

#include "ModeDefault.h"

#include <Session.h>
#include <lib/dataflow/Node.h>
#include <graph/Graph.h>
#include <graph/renderer/Canvas.h>
#include <graph/renderer/RendererNode.h>
#include <lib/xhui/dialogs/FileSelectionDialog.h>
#include <lib/os/msg.h>
#include <lib/xhui/Theme.h>
#include <lib/base/iter.h>
#include <lib/profiler/Profiler.h>
#include <view/ArtemisWindow.h>
#include <view/DrawingHelper.h>
#include <view/MultiView.h>
#include "storage/Storage.h"

ModeDefault::ModeDefault(Session* s) : Mode(s) {
	multi_view = new MultiView(session);

	data = session->data.get();
	graph = session->graph;

	xhui::run_repeated(0.1f, [this] {
		if (simulation_active)
			graph->iterate_simulation(0.1f);
		if (graph->iterate())
			session->win->request_redraw();
	});
	xhui::run_repeated(2.0f, [this] {
		profiler::next_frame();
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
		artemis::graph::_current_simulation_time_ = 0;
		graph->reset_state();
		update_menu();
	});

	update_menu();
}

void ModeDefault::update_menu() {
	auto win = session->win;
	win->enable("simulation-start", !simulation_active);
	win->enable("simulation-pause", simulation_active or artemis::graph::_current_simulation_time_ > 0);
	win->enable("simulation-stop", simulation_active or artemis::graph::_current_simulation_time_ > 0);
}

artemis::graph::Canvas* get_canvas(dataflow::Graph* graph) {
	for (auto n: graph->nodes)
		if (n->flags & dataflow::NodeFlags::Canvas)
			return static_cast<artemis::graph::Canvas*>(n);
	return nullptr;
}

void ModeDefault::on_draw_win(const RenderParams& params, MultiViewWindow* win, RenderViewData& rvd) {
	if (auto c = get_canvas(graph)) {
		if (c->background().a < 0)
			c->background.set(xhui::Theme::_default.background_low);
		c->draw_win(params, win, rvd);
	} else {
		session->drawing_helper->clear(params, Red);//xhui::Theme::_default.background_low);
	}
}

void ModeDefault::on_draw_post(Painter* p) {
	if (auto c = get_canvas(graph))
		c->draw_2d(p);

	p->set_color(White);
	p->draw_str(p->area().p11() - vec2(100, 50), format("t = %.1f", artemis::graph::_current_simulation_time_));

	if (show_profiling) {
		auto report = profiler::digest_report(profiler::previous_frame_timing);
		for (const auto& [i, c]: enumerate(report))
			p->draw_str(p->area().p00() + vec2(20, 20 + 20 * (float)i), format("%s  %.2f  %.2f  %.d", profiler::get_name(c.channel), c.average * 1000, c.total * 1000, c.count));
	}
}

void ModeDefault::on_command(const string& id) {
	if (id == "save") {
		session->storage->auto_save(data);
	}
	if (id == "save-as") {
		session->storage->save_as(data);
	}
}

void ModeDefault::on_key_down(int key) {
	if (key == xhui::KEY_CONTROL | key == xhui::KEY_T)
		show_profiling = !show_profiling;
}




