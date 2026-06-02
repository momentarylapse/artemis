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
#include <lib/xhui/config.h>
#include <lib/os/msg.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/xhui/xhui.h>
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
#if 0
	if (mode_world)
		delete mode_world;
	/*delete mode_material;
	delete mode_model;
	delete mode_font;
	delete mode_admin;*/

	if (multi_view_2d)
		delete multi_view_2d;
	if (multi_view_3d)
		delete multi_view_3d;
	// saving the configuration data...
	if (storage) {
		hui::config.set_str("RootDir", storage->root_dir.str());
		hui::config.set_str("Language", hui::get_cur_language());
		/*HuiConfig.set_bool("LocalDocumentation", LocalDocumentation);
		HuiConfig.set_str("WorldScriptVarFile", WorldScriptVarFile);
		HuiConfig.set_str("ObjectScriptVarFile", ObjectScriptVarFile);
		HuiConfig.set_str("ItemScriptVarFile", ItemScriptVarFile);*/
		//HuiConfig.set_int("UpdateNormalMaxTime (ms)", int(UpdateNormalMaxTime * 1000.0f));
		hui::config.save(app->directory | "config.txt");
		delete storage;
	}
#endif


	app->end();
}


void Session::remove_message() {
	messages.erase(0);
	win->request_redraw();
}

void Session::add_message(Message::Type type, const string &message) {
	msg_write(message.replace("<b>", "").replace("</b>", ""));
	messages.add({type, message});
	win->request_redraw();
	xhui::run_later(3.0f, [this] {
		msg_write("REM");
		remove_message();
	});
}


void Session::error(const string &message) {
	add_message(Message::Type::Error, message);
}

void Session::info(const string &message) {
	add_message(Message::Type::Info, message);
}

void Session::warning(const string &message) {
	add_message(Message::Type::Warning, message);
}



