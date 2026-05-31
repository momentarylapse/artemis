/*
 * Session.cpp
 *
 *  Created on: 12 Sept 2023
 *      Author: michi
 */

#include "Session.h"
#include "Artemis.h"
#include "data/Data.h"
#include "storage/format/Format.h"
#include <processing/helper/GlobalThreadPool.h>
#include <storage/Storage.h>
#include <lib/xhui/config.h>
#include <lib/image/image.h>
#include <lib/os/msg.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/yrenderer/target/XhuiRenderer.h>
#include <lib/yrenderer/helper/LineHelper.h>
#include <lib/yrenderer/TextureManager.h>
#include <lib/yrenderer/Context.h>
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

void init_graphics_stuff(Session* session, xhui::Painter* pp) {

	auto dir = artemis::PluginManager::directory() | "static";
	session->texture_manager = new yrenderer::TextureManager(session->ctx->context, dir);
	session->shader_manager = new yrenderer::ShaderManager(session->ctx->context, dir | "shader");
	session->ctx->shader_manager = session->shader_manager;
	session->ctx->texture_manager = session->texture_manager;
	session->shader_manager->default_shader = "default.shader";
	session->material_manager = new yrenderer::MaterialManager(session->ctx->texture_manager, "");
	session->ctx->material_manager = session->material_manager;
	session->line_helper = new yrenderer::LineHelper();
	try {
			session->shader_manager->load_shader_module("module-basic-data.shader");
			session->shader_manager->load_shader_module("module-basic-interface.shader");
			session->shader_manager->load_shader_module("module-vertex-default.shader");
			session->shader_manager->load_shader_module("module-vertex-animated.shader");
			session->shader_manager->load_shader_module("module-light-sources-default.shader");
			session->shader_manager->load_shader_module("module-shadows-pcf.shader");
			session->shader_manager->load_shader_module("module-lighting-pbr.shader");
			session->shader_manager->load_shader_module("forward/module-surface.shader");
		} catch(Exception& e) {
				msg_error(e.message());
			}

	//		engine.file_errors_are_critical = false;
	//		engine.ignore_missing_files = true;
	//		engine.resource_manager = session->resource_manager;
}

Session *create_session(bool with_window) {
	auto s = new Session;
	s->storage = new Storage(s);
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

	storage = nullptr;
	shader_manager = nullptr;
	texture_manager = nullptr;
	material_manager = nullptr;
	win = nullptr;
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


base::future<void> Session::allow_termination() {
	base::promise<void> promise;
#if 0

	if (!cur_mode) {
		promise();
		return promise.get_future();
	}
	Data *d = cur_mode->get_data();
	if (!d) {
		promise();
		return promise.get_future();
	}
	if (d->action_manager->is_save()) {
		promise();
		return promise.get_future();
	}
	hui::question_box(win,_("Quite a polite question"),_("You increased entropy. Do you wish to save your work?"), true)
		.then([promise] (bool answer) mutable {
			if (!answer) {
				promise();
			} else {
				//bool saved = cur_mode->save();
				//return saved;
				promise.fail();
			}
		}).on_fail([promise] () mutable {
			promise.fail();
		});
#else
	promise();
#endif
	return promise.get_future();
}


