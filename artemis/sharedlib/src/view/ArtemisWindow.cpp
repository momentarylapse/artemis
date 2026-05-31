//
// Created by michi on 19.01.25.
//

#include "ArtemisWindow.h"
#include "Mode.h"
#include "lib/xhui/xhui.h"
#include "lib/xhui/Painter.h"
#include <lib/xhui/Context.h>
#include <lib/xhui/Dialog.h>
#include <lib/xhui/TextLayout.h>
#include <lib/xhui/controls/Toolbar.h>
#include <lib/xhui/dialogs/FileSelectionDialog.h>
#include <lib/xhui/dialogs/QuestionDialog.h>
#include <lib/xhui/dialogs/ColorSelectionDialog.h>
#include <data/Data.h>
#include <lib/xhui/controls/DrawingArea.h>
#include <lib/yrenderer/Context.h>
#include <lib/yrenderer/helper/LineHelper.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/os/msg.h>
#include <lib/xhui/Theme.h>
#include <lib/yrenderer/ShaderManager.h>
#include <lib/yrenderer/MaterialManager.h>
#include <lib/yrenderer/TextureManager.h>
#include <lib/yrenderer/Renderer.h>

#include "Artemis.h"
#include "Session.h"
#include "graph/Graph.h"
#include "lib/base/iter.h"
#include "plugins/PluginManager.h"
#include "storage/Storage.h"


extern string AppName;


//Session* session;
namespace artemis {
	Session* default_session;
}

rect dynamicly_scaled_area(ygfx::FrameBuffer*) { return {}; }
rect dynamicly_scaled_source() { return {}; }


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


ArtemisWindow::ArtemisWindow(Session* _session) : obs::Node<xhui::Window>(AppName, 1024, 768),
	in_redraw(this, [this] {
		request_redraw();
	}),
	in_data_selection_changed(this, [this] {
		//msg_write("SEL CHANGED");
		request_redraw();
		update_menu();
	}),
	in_data_changed(this, [this] {
		//msg_write("DATA CHANGED");
		//session->cur_mode->on_set_multi_view();
		//session->cur_mode->multi_view->force_redraw();
		request_redraw();
		update_menu();
	}),
	in_action_failed(this, [this] {
		auto am = session->data->action_manager;
		session->error(format("Action failed: %s\nReason: %s", am->error_location.c_str(), am->error_message.c_str()));
	}),
	in_saved(this, [this] {
		session->info("Saved!");
		update_menu();
	})
{
	maximize(true);
	session = _session;
	artemis::default_session = session;

	from_source(R"foodelim(
Dialog x x padding=0
	Overlay ? ''
		Grid main-grid '' spacing=0
			Grid left-grid '' spacing=0
				.
				---|
				Overlay code-editor-panel '' visible=no
					Grid code-editor-grid '' greedfactory=0.5
					Grid overlay-code-grid '' margin=25
						Grid ? ''
							Label ? '' ignorehover expandx
							Button close-code-editor 'X' noexpandx
						---|
						Label ? '' ignorehover expandy
						---|
						Grid overlay-button-grid-code-bottom '' spacing=20
							Button code-run 'Run' image=media-playback-start-symbolic height=50 width=50 padding=7 noexpandx ignorefocus
			Overlay ? '' width=400 greedfactorx=1.6 expandx
				Grid canvas-grid '' grabfocus
				DrawingArea canvas-overlay '' ignorehover expandx
				Grid overlay-main-grid '' margin=25
					Label ? '' ignorehover expandy
					---|
					Grid overlay-button-grid-bottom '' spacing=20
						Label ? '' ignorehover expandx
						Button simulation-start 'Start' image=media-playback-start-symbolic height=50 width=50 padding=7 noexpandx ignorefocus
						Button simulation-pause 'Pause' image=media-playback-pause-symbolic height=50 width=50 padding=7 noexpandx ignorefocus
						Button simulation-stop 'Stop' image=media-playback-stop-symbolic height=50 width=50 padding=7 noexpandx ignorefocus
						Button simulation-step 'Step' height=50 width=50 padding=7 noexpandx ignorefocus
		DrawingArea overlay-area '' ignorehover
)foodelim");
	Panel::set_option("padding", "0");

	//toolbar = (xhui::Toolbar*)get_control("toolbar");

#ifdef OS_MAC
	int mod = xhui::KEY_SUPER;
#else
	int mod = xhui::KEY_CONTROL;
#endif

	set_key_code("open", mod + xhui::KEY_O);
	set_key_code("save", mod + xhui::KEY_S);
	set_key_code("save-as", mod + xhui::KEY_SHIFT + xhui::KEY_S);
	set_key_code("exit", mod + xhui::KEY_Q);
	set_key_code("undo", mod + xhui::KEY_Z);
	set_key_code("redo", mod + xhui::KEY_Y);
	set_key_code("show-profiling", mod + xhui::KEY_T);
	event("exit", [this] {
		request_destroy();
	});

	event_xp("canvas-overlay", xhui::event_id::Initialize, [this] (Painter* p) {
		auto pp = (xhui::Painter*)p;
		session->ctx = yrenderer::api_init_xhui(pp);
		init_graphics_stuff(session, pp);

		session->promise_started(session);
	});



	event("close-code-editor", [this] {
		set_visible("code-editor-panel", false);
	});
	event_xp("overlay-area", xhui::event_id::Draw, [this] (Painter* p) {
		for (const auto& [i, m]: enumerate(session->messages)) {
			color fg = xhui::Theme::_default.text_label;
			color bg = xhui::Theme::_default.background_button;
			string text = m.text;
			if (m.type == Session::Message::Type::Error) {
				//bg = color(1, 0.5f, 0, 0);
				bg = color(1, 0.5f, 0.3f, 0);
				text = "<b>ERROR</b>: " +  m.text;
			} else if (m.type == Session::Message::Type::Warning) {
				bg = color(1, 0.5f, 0.3f, 0);
			}
			const auto l = xhui::TextLayout::from_format_string(p, text, xhui::Theme::_default.font_size * 1.5f);
			xhui::draw_text_layout_with_box(p, p->area().center() + vec2(-l.box.width()/2, 30.0f*(float)i), l, fg, bg, 14, 10);
		}
	});
	event_x(id, xhui::event_id::Close, [this] {
	});
}

void ArtemisWindow::on_key_down(int key) {
}

void ArtemisWindow::update_menu() {
}



