//
// Created by Michael Ankele on 2025-03-12.
//

#include "DefaultNodePanel.h"
#include <data/util/ColorMap.h>
#include <lib/dataflow/Node.h>
#include <lib/dataflow/Setting.h>
#include <lib/base/iter.h>
#include <lib/kaba/syntax/Class.h>
#include <lib/any/conversion.h>
#include <lib/os/msg.h>
#include <lib/xhui/xhui.h>

#include "Session.h"
#include "dialog/ColorMapDialog.h"
#include "graph/Graph.h"
#include "lib/kaba/dynamic/dynamic.h"

namespace kaba {
	extern const Class* TypeFloat64List;
}

void draw_color_map_background(Painter* p, const artemis::data::ColorMap& color_map, float value_min, float value_max, const rect& area);

DefaultNodePanel::DefaultNodePanel(Session* s, dataflow::Node* n) : xhui::Panel("node-panel") {
	session = s;
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
		if (s->type == kaba::TypeFloat64) {
			auto ss = node->settings[i]->as<double>();
			add_control("SpinButton", "", 1, i, id);
			set_options(id, "expandx");
			if (s->options.find("range=") == 0)
				set_options(id, s->options);
			else
				set_options(id, "range=::0.001");
			set_float(id, (float)(*ss)());
			event(id, [this, id, ss] {
				session->data->node_set_setting(node, ss->name, get_float(id));
				//ss->set((double)get_float(id));
			});
		} else if (s->type == kaba::TypeInt32) {
			auto ss = node->settings[i]->as<int>();
			if (s->options.head(4) == "set=") {
				auto xx = s->options.sub(5, -1).parse_tokens(":,");
				add_control("ComboBox", "", 1, i, id);
				set_options(id, "expandx");
				Array<int> values;
				for (int si=0; si<(xx.num + 2)/4; si++) {
					values.add(xx[si*4]._int());
					add_string(id, xx[si*4 + 2]);
				}
				int index = values.find((*ss)());
				if (index >= 0)
					set_int(id, index);
				event(id, [this, id, ss, values] {
					session->data->node_set_setting(node, ss->name, values[get_int(id)]);
					//ss->set(values[get_int(id)]);
				});
			} else {
				add_control("SpinButton", "", 1, i, id);
				set_options(id, "expandx");
				if (s->options.find("range=") == 0)
					set_options(id, s->options);
				set_int(id, (*ss)());
				event(id, [this, id, ss] {
					session->data->node_set_setting(node, ss->name, get_int(id));
					//ss->set(get_int(id));
				});
			}
		} else if (s->type == kaba::TypeString) {
			auto ss = node->settings[i]->as<string>();
			add_control("Edit", "", 1, i, id);
			set_options(id, "expandx");
			set_string(id, (*ss)());
			event(id, [this, id, ss] {
				session->data->node_set_setting(node, ss->name, get_string(id));
				//ss->set(get_string(id));
			});
		} else if (s->type == kaba::TypeBool) {
			auto ss = node->settings[i]->as<bool>();
			add_control("CheckBox", "", 1, i, id);
			set_options(id, "expandx");
			check(id, (*ss)());
			event(id, [this, id, ss] {
				session->data->node_set_setting(node, ss->name, is_checked(id));
				//ss->set(is_checked(id));
			});
		} else if (s->type == kaba::TypeColor) {
			auto ss = node->settings[i]->as<color>();
			add_control("ColorButton", "", 1, i, id);
			set_options(id, "expandx");
			set_color(id, (*ss)());
			event(id, [this, id, ss] {
				session->data->node_set_setting(node, ss->name, color_to_any(get_color(id)));
				//ss->set(get_color(id));
			});
		} else if (s->type->name == "ColorMap") {
			auto ss = node->settings[i]->as<artemis::data::ColorMap>();
			add_control("DrawingArea", "", 1, i, id);
			set_options(id, "expandx");
			event_xp(id, xhui::event_id::Draw, [this, id, ss] (Painter* p) {
				const auto cm = (*ss)();
				draw_color_map_background(p, cm, cm.min(), cm.max(), p->area());
			});
			event_x(id, xhui::event_id::LeftButtonUp, [this, ss] {
				ColorMapDialog::ask(this, (*ss)()).then([this, ss] (const artemis::data::ColorMap& cm) {
					session->data->node_set_setting(node, ss->name, kaba::dynify(&cm, dataflow::get_class<artemis::data::ColorMap>()));
					//ss->set(cm);
				});
			});
		} else if (s->type == kaba::TypeFloat64List) {
			auto ss = node->settings[i]->as<Array<double>>();
			add_control("Edit", "", 1, i, id);
			set_options(id, "expandx");
			enable(id, false);
			set_string(id, str((*ss)()));
		} else {
			add_control("Label", format("(%s)", s->type->name), 1, i, "");
		}
	}
	if (node->settings.num == 0)
		add_control("Label", "(no settings)", 0, 0, "");
	size_mode_x = SizeMode::Shrink;
	size_mode_y = SizeMode::Shrink;
}

