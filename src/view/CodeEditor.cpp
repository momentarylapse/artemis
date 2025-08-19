//
// Created by michi on 8/19/25.
//

#include "CodeEditor.h"
#include <lib/xhui/Panel.h>
#include <lib/kaba/kaba.h>
#include <lib/os/msg.h>

CodeEditor::CodeEditor(xhui::Panel* _panel, const string& _id, const Path& _package_dir) {
	panel = _panel;
	id = _id;
	package_dir = _package_dir;

	panel->set_string(id, R"foodelim(use artemis.*

func main()
	var g = weak(current_session().graph)
	print("hi")
)foodelim");
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
		msg_error(e.message());
	}
}


