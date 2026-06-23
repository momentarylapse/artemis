/*
 * Session.cpp
 *
 *  Created on: 12 Sept 2023
 *      Author: michi
 */

#include "Session.h"
#include "Artemis.h"
#include <lib/history/Data.h>
#include <processing/helper/GlobalThreadPool.h>
#include <lib/os/msg.h>
#include <lib/ygraphics/graphics-impl.h>
#include "graph/Graph.h"
#include <plugins/PluginManager.h>

Session* _current_session_ = nullptr;

namespace yrenderer {
rect dynamicly_scaled_area(ygfx::FrameBuffer *fb) {
	return rect(0, fb->width, 0, fb->height);
}
rect dynamicly_scaled_source() {
	return rect::ID;
}
}

Session* create_session() {
	auto s = new Session;
	s->data = new artemis::graph::DataGraph(s);
	s->graph = s->data->graph.get();
	_current_session_ = s;
	artemis::processing::pool::init();
	return s;
}

base::future<Session*> emit_new_session() {
	auto s = create_session();
	return s->promise_started.get_future();
}

bool session_is_empty(Session *s) {
	return false;
}

base::future<Session*> emit_empty_session(Session* parent) {
	if (session_is_empty(parent))
		return parent->promise_started.get_future();

	return emit_new_session();
}

Session::Session() {
	ctx = nullptr;
	shader_manager = nullptr;
	texture_manager = nullptr;
	material_manager = nullptr;
	win = nullptr;
	graph = nullptr;
	t = 0;
	dt = 0.1;
}

Session::~Session() {
}




