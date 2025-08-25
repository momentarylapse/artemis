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

class Session;

class CodeEditor {
public:
	CodeEditor(Session* s, xhui::Panel* panel, const string& id, const Path& package_dir);

	void update_markup();

	void run();

	Session* session;
	xhui::Panel* panel;
	xhui::Edit* edit;
	string id;
	Path package_dir;
};

