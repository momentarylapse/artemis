//
// Created by michi on 8/19/25.
//

#include "CodeEditor.h"
#include <lib/xhui/Panel.h>
#include <lib/xhui/xhui.h>
#include <lib/kaba/kaba.h>
#include <lib/codeeditor/CodeEditor.h>
#include <lib/os/msg.h>

#include "Session.h"

namespace artemis::view {

CodeEditor::CodeEditor(Session* _session, xhui::Panel* panel, const string& id, const Path& _package_dir) {
	session = _session;
	package_dir = _package_dir;

	editor = new codeedit::CodeEditor();
	panel->embed(id, 0, 0, editor);
	editor->filename = "dummy.kaba";
	editor->set_visible("structure", false);

	editor->set_string(editor->id_edit, R"foodelim(use artemis.*

func main()
	var g = weak(current_session().graph)
	print("hi")
)foodelim");
	editor->update_highlight_all();

	editor->out_info >> create_data_sink<string>([this] (const string& m) {
		session->info(m);
	});
	editor->out_error >> create_data_sink<string>([this] (const string& m) {
		session->error(m);
	});

}


void CodeEditor::run() {
	string source = editor->edit->text;

	try {
		kaba::config.default_filename = package_dir | "from-code-editor.kaba";
		auto m = kaba::default_context->dll_create_module_for_source(source);
		typedef void t_f();
		if (auto f = (t_f*)m->match_function("main", "void", {}))
			(*f)();
	} catch (kaba::Exception& e) {
		session->error(e.message());
	}
}

}


