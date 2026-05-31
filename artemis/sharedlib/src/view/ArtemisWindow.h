//
// Created by michi on 19.01.25.
//

#pragma once

#include <lib/xhui/xhui.h>
#include <lib/pattern/Observable.h>

namespace xhui {
	class Toolbar;
}
class Session;

class ArtemisWindow : public obs::Node<xhui::Window> {
public:
	Session* session;
	Array<string> args;

	obs::sink in_redraw;
	obs::sink in_data_selection_changed;
	obs::sink in_data_changed;
	obs::sink in_action_failed;
	obs::sink in_saved;

	xhui::Toolbar* toolbar;

	explicit ArtemisWindow(Session* session);

	void on_key_down(int key) override;

	void update_menu();
};
