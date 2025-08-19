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

#include "lib/xhui/config.h"
#include "storage/Storage.h"

ModeDefault::ModeDefault(Session* s) : Mode(s) {
	multi_view = new MultiView(session);

	if (xhui::config.get_bool("debug", false))
		show_profiling = true;

	data = session->data.get();
	graph = session->graph;

	xhui::run_repeated(session->simulation_update_dt, [this] {
		if (simulation_active)
			graph->iterate_simulation();
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
		session->graph->t = 0;
		graph->reset_state();
		update_menu();
	});

	update_menu();
}

void ModeDefault::update_menu() {
	auto win = session->win;
	win->enable("simulation-start", !simulation_active);
	win->enable("simulation-pause", simulation_active or session->graph->t > 0);
	win->enable("simulation-stop", simulation_active or session->graph->t > 0);
}

artemis::graph::Canvas* get_canvas(dataflow::Graph* graph) {
	for (auto n: graph->nodes)
		if (n->flags & dataflow::NodeFlags::Canvas)
			return static_cast<artemis::graph::Canvas*>(n);
	return nullptr;
}

void ModeDefault::on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win, yrenderer::RenderViewData& rvd) {
	if (auto c = get_canvas(graph)) {
		if (c->background().a < 0)
			c->background.set(xhui::Theme::_default.background_low);
		c->draw_win(params, win, rvd);
	} else {
		session->drawing_helper->clear(params, Red);//xhui::Theme::_default.background_low);
	}
}

string nice_time(float t, float dt) {
	if (dt < 0.0001f)
		return format("%.2f us", t * 1000000.0f);
	if (dt < 0.001f)
		return format("%.3f ms", t * 1000.0f);
	if (dt < 0.01f)
		return format("%.3f s", t);
	return format("%.2f s", t);
}

void ModeDefault::on_draw_post(Painter* p) {
	if (auto c = get_canvas(graph))
		c->draw_2d(p);

	auto draw_str_right = [p] (const vec2& pos, const string& str) {
		float w = p->get_str_width(str);
		p->draw_str(pos - vec2(w, 0), str);
	};

	p->set_color(xhui::Theme::_default.text_label);
	p->set_font_size(xhui::Theme::_default.font_size * 1.3f);
	draw_str_right(p->area().p11() - vec2(320, 55), format("t = %s", nice_time(session->graph->t, session->graph->dt)));
	p->set_font_size(xhui::Theme::_default.font_size);

	if (show_profiling) {
		p->set_color(xhui::Theme::_default.text_disabled);
		p->set_font_size(xhui::Theme::_default.font_size * 0.85f);
		auto report = profiler::digest_report(profiler::previous_frame_timing);
		for (const auto& [i, c]: enumerate(report)) {
			draw_str_right(p->area().p00() + vec2(140, 20 + 18 * (float)i), profiler::get_name(c.channel));
			draw_str_right(p->area().p00() + vec2(180, 20 + 18 * (float)i), format("%.2f", c.average * 1000));
			draw_str_right(p->area().p00() + vec2(220, 20 + 18 * (float)i), format("%.2f", c.total * 1000));
			draw_str_right(p->area().p00() + vec2(260, 20 + 18 * (float)i), format("%.d", c.count));
		}
		p->set_font_size(xhui::Theme::_default.font_size);
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




