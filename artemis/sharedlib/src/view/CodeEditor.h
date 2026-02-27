//
// Created by michi on 8/19/25.
//

#pragma once

#include <lib/base/base.h>
#include <lib/os/path.h>

namespace xhui {
	class Panel;
	class Edit;
}
namespace codeedit {
	class CodeEditor;
}

class Session;

namespace artemis::view {

class CodeEditor {
public:
	CodeEditor(Session* s, xhui::Panel* panel, const string& id, const Path& package_dir);

	void run();

	Session* session;
	codeedit::CodeEditor* editor;
	Path package_dir;
};

}

