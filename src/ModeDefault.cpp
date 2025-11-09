//
// Created by michi on 09.03.25.
//

#include "ModeDefault.h"
#include <Session.h>
#include <graph/Graph.h>
#include <graph/canvas/Canvas.h>
#include <lib/dataflow/Node.h>
#include <lib/xhui/dialogs/FileSelectionDialog.h>
#include <lib/xhui/Theme.h>
#include <lib/xhui/config.h>
#include <lib/base/iter.h>
#include <lib/os/msg.h>
#include <lib/profiler/Profiler.h>
#include <view/ArtemisWindow.h>
#include <view/DrawingHelper.h>
#include <storage/Storage.h>

ModeDefault::ModeDefault(Session* s) : Mode(s) {
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
		session->t = 0;
		session->graph->reset_state();
		update_menu();
	});
	win->event("simulation-step", [this] {
		simulation_active = false;
		graph->iterate_simulation();
		update_menu();
	});

	update_menu();
}

void ModeDefault::update_menu() {
	auto win = session->win;
	win->enable("simulation-start", !simulation_active);
	win->enable("simulation-pause", simulation_active or session->t > 0);
	win->enable("simulation-stop", simulation_active or session->t > 0);
}

artemis::graph::Canvas* get_canvas(dataflow::Graph* graph) {
	for (auto n: graph->nodes)
		if (n->flags & dataflow::NodeFlags::Canvas)
			return static_cast<artemis::graph::Canvas*>(n);
	return nullptr;
}

string nice_time(double t, double dt) {
	Array<string> s;
	if (t >= 3600)
		s.add(format("%d h", (int)(t / 3600)));

	if (t >= 60) {
		if (t >= 3600)
			s.add(format("%02d min", (int)(t / 60) % 60));
		else
			s.add(format("%d min", (int)(t / 60) % 60));
	}
	if (t >= 1) {
		if (t >= 60)
			s.add(format("%02d s", (int)t % 60));
		else
			s.add(format("%d s", (int)t % 60));
	}
	if (t >= 0.001 and dt < 1) {
		if (t >= 1)
			s.add(format("%03d ms", (int)(t * 1000) % 1000));
		else
			s.add(format("%d ms", (int)(t * 1000) % 1000));
	}
	if (t >= 0.000001 and dt < 0.001) {
		if (t >= 0.001)
			s.add(format("%03d us", (int)(t * 1000000) % 1000));
		else
			s.add(format("%d us", (int)(t * 1000000) % 1000));
	}
	if (dt < 0.000001) {
		if (t >= 0.000001)
			s.add(format("%03d ns", (int)(t * 1000000000) % 1000));
		else
			s.add(format("%d ns", (int)(t * 1000000000) % 1000));
	}
	if (s.num == 0)
		return "0 s";
	return implode(s, " ");
}

void ModeDefault::on_draw_post(Painter* p) {
	auto draw_str_right = [p] (const vec2& pos, const string& str) {
		float w = p->get_str_width(str);
		p->draw_str(pos - vec2(w, 0), str);
	};

	p->set_color(xhui::Theme::_default.text_label);
	p->set_font_size(xhui::Theme::_default.font_size * 1.3f);
	draw_str_right(p->area().p11() - vec2(320, 55), format("t = %s", nice_time(session->t, session->dt)));
	p->set_font_size(xhui::Theme::_default.font_size);

	if (show_profiling) {
		p->set_font_size(xhui::Theme::_default.font_size * 0.85f);
		auto report = profiler::digest_report(profiler::previous_frame_timing);
		for (const auto& [i, c]: enumerate(report)) {
			float fraction = c.total / max(profiler::previous_frame_timing.total_time, 0.01f);
			if (fraction > 0.03)
				p->set_color(xhui::Theme::_default.text);
			else
				p->set_color(xhui::Theme::_default.text_disabled);
			draw_str_right(p->area().p00() + vec2(140, 20 + 18 * (float)i), profiler::get_name(c.channel));
			draw_str_right(p->area().p00() + vec2(190, 20 + 18 * (float)i), format("%.1f%%", fraction * 100));
			draw_str_right(p->area().p00() + vec2(230, 20 + 18 * (float)i), format("%.2f", c.average * 1000));
			draw_str_right(p->area().p00() + vec2(270, 20 + 18 * (float)i), format("%.d", c.count));
		}
		p->set_font_size(xhui::Theme::_default.font_size);
	}
}

void ModeDefault::on_command(const string& id) {
	if (id == "save") {
		session->storage->auto_save(data).then([this] {
			session->info("saved");
		});
	}
	if (id == "save-as") {
		session->storage->save_as(data).then([this] {
			session->info("saved");
		});
	}
	if (id == "show-profiling") {
		show_profiling = !show_profiling;
		session->win->request_redraw();
	}
}

void ModeDefault::on_key_down(int key) {
}




