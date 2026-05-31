/*
 * Session.cpp
 *
 *  Created on: 12 Sept 2023
 *      Author: michi
 */

#include "Session.h"
#include "view/ArtemisWindow.h"
#include "Artemis.h"
#include "view/Mode.h"
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
#include "graph/Graph.h"

Session* _current_session_ = nullptr;

Session *create_session(bool with_window) {
	auto s = new Session;
	s->storage = new Storage(s);
	s->data = new artemis::graph::DataGraph(s);
	s->graph = s->data->graph.get();
	if (with_window) {
		s->win = new ArtemisWindow(s);
		xhui::fly(s->win);
	}
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

#ifdef ksdjfhskjdfhkjsdhfj
void Session::create_initial_resources(yrenderer::Context *_ctx) {

	ctx = _ctx;

	// initialize engine
	resource_manager = new ResourceManager(ctx, "", "", "");
///	drawing_helper = new DrawingHelper(ctx, resource_manager, app->directory_static);

	engine.ignore_missing_files = true;
	engine.set_context(ctx, resource_manager);
	resource_manager->shader_manager->load_shader("module-vertex-default.shader");
	//ResourceManager::default_shader

	CameraInit();
	GodInit(0);

#if 0
	multi_view_3d = new MultiView::MultiView(this, true);
	multi_view_2d = new MultiView::MultiView(this, false);
	mode_model = new ModeModel(this, multi_view_3d, multi_view_2d);
	mode_world = new ModeWorld(this, multi_view_3d);
	mode_font = new ModeFont(this, multi_view_2d);
	mode_admin = new ModeAdministration(this);
#endif

	storage = new Storage(this);
	storage->set_root_directory(xhui::config.get_str("RootDir", ""));

#if 0
	mode_material = new ModeMaterial(this, multi_view_3d);

	/*mmodel->FFVBinary = mobject->FFVBinary = mitem->FFVBinary = mmaterial->FFVBinary = mworld->FFVBinary = mfont->FFVBinary = false;
	mworld->FFVBinaryMap = true;*/

	multi_view_3d->out_settings_changed >> create_sink([this] {
		win->update_menu();
	});
	multi_view_3d->out_selection_changed >> create_sink([this] {
		cur_mode->on_selection_change();
		win->update_menu();
	});
	multi_view_3d->out_viewstage_changed >> create_sink([this] {
		cur_mode->on_view_stage_change();
		win->update_menu();
	});
	multi_view_3d->out_redraw >> create_sink([this] {
		win->redraw("nix-area");
	});

	multi_view_2d->out_settings_changed >> create_sink([this]{
		win->update_menu();
	});
	multi_view_2d->out_selection_changed >> create_sink([this] {
		cur_mode->on_selection_change();
		win->update_menu();
	});
	multi_view_2d->out_viewstage_changed >> create_sink([this] {
		cur_mode->on_view_stage_change();
		win->update_menu();
	});
	multi_view_2d->out_redraw >> create_sink([this] {
		win->redraw("nix-area");
	});
#endif

	promise_started(this);
}
#endif

// do we change roots?
//  -> data loss?
base::future<void> mode_switch_allowed(Mode *m) {
//	if (!m->session->cur_mode or m->equal_roots(m->session->cur_mode)) {
		base::promise<void> promise;
		promise();
		return promise.get_future();
//	} else {
//		return m->session->allow_termination();
//	}
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


