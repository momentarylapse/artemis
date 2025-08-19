//
// Created by michi on 8/19/25.
//

#pragma once

#include <lib/base/base.h>
#include <lib/os/path.h>

namespace xhui {
	class Panel;
}

class CodeEditor {
public:
	CodeEditor(xhui::Panel* panel, const string& id, const Path& package_dir);

	void run();

	xhui::Panel* panel;
	string id;
	Path package_dir;
};

