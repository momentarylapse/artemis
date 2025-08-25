//
// Created by michi on 8/19/25.
//

#include "CodeEditor.h"
#include <lib/xhui/Panel.h>
#include <lib/xhui/controls/Edit.h>
#include <lib/xhui/xhui.h>
#include <lib/kaba/kaba.h>
#include <lib/syntaxhighlight/BaseParser.h>
#include <lib/syntaxhighlight/Theme.h>
#include <lib/os/msg.h>

#include "Session.h"

CodeEditor::CodeEditor(Session* _session, xhui::Panel* _panel, const string& _id, const Path& _package_dir) {
	session = _session;
	panel = _panel;
	id = _id;
	package_dir = _package_dir;

	//panel->set_options(id, "fontsize=50,lineheightscale=2.0");
	//panel->set_options(id, "fontsize=16,lineheightscale=1.1,linenumbers,focusframe=no");

	panel->set_string(id, R"foodelim(use artemis.*

func main()
	var g = weak(current_session().graph)
	print("hi")
)foodelim");

	edit = static_cast<xhui::Edit*>(panel->get_control(id));


	static int xcounter = 0;
	panel->event_x(id, xhui::event_id::Changed, [this] {
		xcounter ++;
		xhui::run_later(2.0f, [this] {
			xcounter --;
			if (xcounter == 0)
				update_markup();
		});
	});

	update_markup();
}

void CodeEditor::update_markup() {
	if (auto p = GetParser("a.kaba")) {
		edit->clean_markup(0, edit->text.num);
		p->prepare_symbols(edit->text, "a.kaba");
		for (const auto& m: p->create_markup(edit->text, 0)) {
			const auto& c = syntaxhighlight::default_theme->context[(int)m.type];
			edit->add_markup({m.start, m.end, c.bold ? xhui::FontFlags::Bold : xhui::FontFlags::None, c.fg});
		}
	}
}


void CodeEditor::run() {
	string source = panel->get_string(id);

	try {
		kaba::config.default_filename = package_dir | "from-code-editor.kaba";
		auto m = kaba::default_context->create_module_for_source(source);
		typedef void t_f();
		auto f = (t_f*)m->match_function("main", "void", {});
		if (f)
			(*f)();
	} catch (kaba::Exception& e) {
		session->error(e.message());
	}
}


