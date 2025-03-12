//
// Created by Michael Ankele on 2025-03-12.
//

#include "DefaultNodePanel.h"
#include <graph/Node.h>
#include <graph/Setting.h>
#include <lib/base/iter.h>
#include <lib/kaba/syntax/Class.h>


DefaultNodePanel::DefaultNodePanel(graph::Node* n) : xhui::Panel("node-panel") {
	node = n;
	from_source(R"foodelim(
Dialog x ''
	Grid node-panel '' class=card
		Group node-group 'Test'
			Grid settings-grid ''
)foodelim");

	set_string("node-group", node->name);

	for (const auto& [i, s]: enumerate(node->settings)) {
		set_target("settings-grid");
		add_control("Label", s->name, 0, i, "");
		string id = format("setting-%d", i);
		if (s->class_ == kaba::TypeFloat32) {
			auto ss = node->settings[i]->as<float>();
			add_control("SpinButton", "", 1, i, id);
			set_options(id, "expandx");
			set_options(id, "range=::0.001");
			set_float(id, (*ss)());
			event(id, [this, id, ss] {
				ss->set(get_float(id));
			});
		} else if (s->class_ == kaba::TypeInt32) {
			auto ss = node->settings[i]->as<int>();
			add_control("SpinButton", "", 1, i, id);
			set_options(id, "expandx");
			set_int(id, (*ss)());
			event(id, [this, id, ss] {
				ss->set(get_int(id));
			});
		} else if (s->class_ == kaba::TypeString) {
			auto ss = node->settings[i]->as<string>();
			add_control("Edit", "", 1, i, id);
			set_options(id, "expandx");
			set_string(id, (*ss)());
			event(id, [this, id, ss] {
				ss->set(get_string(id));
			});
		} else if (s->class_ == kaba::TypeBool) {
			auto ss = node->settings[i]->as<bool>();
			add_control("CheckBox", "", 1, i, id);
			set_options(id, "expandx");
			check(id, (*ss)());
			event(id, [this, id, ss] {
				ss->set(is_checked(id));
			});
		} else if (s->class_ == kaba::TypeColor) {
			auto ss = node->settings[i]->as<color>();
			add_control("ColorButton", "", 1, i, id);
			set_options(id, "expandx");
			set_color(id, (*ss)());
			event(id, [this, id, ss] {
				ss->set(get_color(id));
			});
		} else {
			add_control("Label", format("(%s)", s->class_->name), 1, i, "");
		}
	}
	if (node->settings.num == 0)
		add_control("Label", "(no settings)", 0, 0, "");
	size_mode_x = SizeMode::Shrink;
	size_mode_y = SizeMode::Shrink;
}

