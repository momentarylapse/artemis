//
// Created by michi on 19.01.25.
//

#include "ArtemisWindow.h"
#include "Mode.h"
#include "lib/xhui/xhui.h"
#include "lib/xhui/Painter.h"
#include <lib/xhui/Context.h>
#include <lib/xhui/Dialog.h>
#include <lib/xhui/controls/Toolbar.h>
#include <lib/xhui/dialogs/FileSelectionDialog.h>
#include <lib/xhui/dialogs/QuestionDialog.h>
#include <lib/xhui/dialogs/ColorSelectionDialog.h>
#include <data/Data.h>
#include <lib/xhui/controls/DrawingArea.h>
#include <lib/yrenderer/Context.h>
#include <lib/ygraphics/graphics-impl.h>
#include <y/EngineData.h>
#include "ActionController.h"
#include "DrawingHelper.h"
#include "GraphEditor.h"
#include "CodeEditor.h"
#include "Canvas.h"
#include <lib/os/msg.h>
#include <lib/xhui/Theme.h>
#include <lib/yrenderer/ShaderManager.h>
#include <lib/yrenderer/Renderer.h>
#include "Session.h"
#include "y/helper/ResourceManager.h"
#include "storage/Storage.h"
#include "Session.h"
#include "lib/base/iter.h"
#include "plugins/PluginManager.h"


extern string AppName;


Session* session;

rect dynamicly_scaled_area(ygfx::FrameBuffer*) { return {}; }
rect dynamicly_scaled_source() { return {}; }
void ExternalModelCleanup(Model *m) {}


namespace yrenderer {
	rect dynamicly_scaled_area(ygfx::FrameBuffer *fb) {
		return rect(0, fb->width, 0, fb->height);
	}

	rect dynamicly_scaled_source() {
		return rect::ID;
	}
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
		auto am = session->cur_mode->get_data()->action_manager;
		session->error(format("Action failed: %s\nReason: %s", am->error_location.c_str(), am->error_message.c_str()));
	}),
	in_saved(this, [this] {
		session->info("Saved!");
		update_menu();
	})
{
	maximize(true);
	session = _session;

	from_source(R"foodelim(
Dialog x x padding=0
	Overlay ? ''
		Grid main-grid '' spacing=0
			Grid left-grid '' spacing=0
				.
				---|
				Overlay code-editor-panel '' visible=no
					MultilineEdit code-editor '' greedfactory=0.5 monospace fontsize=15 linenumbers __altbg lineheightscale=1.1 focusframe=no
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

	graph_editor = new artemis::view::GraphEditor(session);
	embed("left-grid", 0, 0, graph_editor);

	code_editor = new artemis::view::CodeEditor(session, this, "code-editor", artemis::PluginManager::directory());


	canvas = new artemis::view::Canvas(session);
	embed("canvas-grid", 0, 0, canvas.get());

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
	event("open", [this] {
		session->cur_mode->on_command("open");
	});
	event("save", [this] {
		session->cur_mode->on_command("save");
	});
	event("save-as", [this] {
		session->cur_mode->on_command("save-as");
	});
	event("exit", [this] {
		request_destroy();
	});
	event("undo", [this] {
		session->cur_mode->on_command("undo");
	});
	event("redo", [this] {
		session->cur_mode->on_command("redo");
	});
	event("show-profiling", [this] {
		session->cur_mode->on_command("show-profiling");
	});

	event_xp("canvas-overlay", xhui::event_id::Initialize, [this] (Painter* p) {
		auto pp = (xhui::Painter*)p;
		session->ctx = yrenderer::api_init_xhui(pp);
		session->resource_manager = new ResourceManager(session->ctx, "", "", "");
		session->ctx->material_manager = session->resource_manager->material_manager;
		session->ctx->shader_manager = session->resource_manager->shader_manager;
		session->ctx->texture_manager = session->resource_manager->texture_manager;
		session->resource_manager->shader_manager->default_shader = "default.shader";
		session->drawing_helper = new DrawingHelper(pp->context, session->ctx);
		try {
			session->resource_manager->shader_manager->load_shader_module("module-basic-data.shader");
			session->resource_manager->shader_manager->load_shader_module("module-basic-interface.shader");
			session->resource_manager->shader_manager->load_shader_module("module-vertex-default.shader");
			session->resource_manager->shader_manager->load_shader_module("module-vertex-animated.shader");
			session->resource_manager->shader_manager->load_shader_module("module-light-sources-default.shader");
			session->resource_manager->shader_manager->load_shader_module("module-shadows-pcf.shader");
			session->resource_manager->shader_manager->load_shader_module("module-lighting-pbr.shader");
			session->resource_manager->shader_manager->load_shader_module("forward/module-surface.shader");
		} catch(Exception& e) {
			msg_error(e.message());
		}

		engine.file_errors_are_critical = false;
		engine.ignore_missing_files = true;
		engine.resource_manager = session->resource_manager;

		session->promise_started(session);
	});
	event_xp("canvas-overlay", xhui::event_id::Draw, [this] (Painter* p) {
		if (session->cur_mode)
			session->cur_mode->on_draw_post(p);
	});



	event("code-run", [this] {
		code_editor->run();
	});
	event("close-code-editor", [this] {
		set_visible("code-editor-panel", false);
	});
	event_xp("overlay-area", xhui::event_id::Draw, [this] (Painter* p) {
		for (const auto& [i, m]: enumerate(session->messages)) {
			color bg = xhui::Theme::_default.background_button;
			string text = m.text;
			if (m.type == Session::Message::Type::Error) {
				//bg = color(1, 0.5f, 0, 0);
				bg = color(1, 0.5f, 0.3f, 0);
				text = "<b>ERROR</b>: " +  m.text;
			} else if (m.type == Session::Message::Type::Warning) {
				bg = color(1, 0.5f, 0.3f, 0);
			}
			const auto l = TextLayout::from_format_string(text, xhui::Theme::_default.font_size * 1.5f);
			DrawingHelper::draw_text_layout_with_box(p, p->area().center() + vec2(-l.box().width()/2, 30.0f*i), l, bg, 14, 10);
		}
	});
	event_x(id, xhui::event_id::Close, [this] {
		if (!session->cur_mode->get_data() or session->cur_mode->get_data()->action_manager->is_save())
			request_destroy();
		else xhui::QuestionDialog::ask(this, "Question", "You have unsaved changes. Do you want to close?").then([this] (xhui::Answer a) {
			if (a == xhui::Answer::Yes)
				request_destroy();
		});
	});
}

void ArtemisWindow::on_key_down(int key) {
}

void ArtemisWindow::update_menu() {
	if (session->cur_mode->get_data()) {
		enable("undo", session->cur_mode->get_data()->action_manager->undoable());
		enable("redo", session->cur_mode->get_data()->action_manager->redoable());
	}
}



