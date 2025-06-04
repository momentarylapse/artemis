//
// Created by Michael Ankele on 2025-03-12.
//

#include "DefaultNodePanel.h"
#include <data/util/ColorMap.h>
#include <lib/dataflow/Node.h>
#include <lib/dataflow/Setting.h>
#include <lib/base/iter.h>
#include <lib/kaba/syntax/Class.h>
#include <lib/os/msg.h>
#include <lib/xhui/xhui.h>

#include "dialog/ColorMapDialog.h"


DefaultNodePanel::DefaultNodePanel(dataflow::Node* n) : xhui::Panel("node-panel") {
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
			if (s->options.find("range=") == 0)
				set_options(id, s->options);
			else
				set_options(id, "range=::0.001");
			set_float(id, (*ss)());
			event(id, [this, id, ss] {
				ss->set(get_float(id));
			});
		} else if (s->class_ == kaba::TypeInt32) {
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
					ss->set(values[get_int(id)]);
				});
			} else {
				add_control("SpinButton", "", 1, i, id);
				set_options(id, "expandx");
				if (s->options.find("range=") == 0)
					set_options(id, s->options);
				set_int(id, (*ss)());
				event(id, [this, id, ss] {
					ss->set(get_int(id));
				});
			}
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
		} else if (s->class_->name == "ColorMap") {
			auto ss = node->settings[i]->as<artemis::data::ColorMap>();
			add_control("DrawingArea", "", 1, i, id);
			set_options(id, "expandx");
			event_xp(id, xhui::event_id::Draw, [this, id, ss] (Painter* p) {
				const auto cm = (*ss)();
				const rect area = p->area();
				const int N = 100;
				for (int k=0; k<N; k++) {
					float t0 = (float)k / (float)N;
					float t1 = (float)(k+1) / (float)N;
					float t = cm.min() + (cm.max() - cm.min()) * t0;
					p->set_color(cm.get(t));
					p->draw_rect(rect(area.x1 + area.width() * t0, area.x1 + area.width() * t1, area.y1, area.y2));
				}
			});
			event_x(id, xhui::event_id::LeftButtonUp, [this, ss] {
				ColorMapDialog::ask(this, (*ss)()).then([ss] (const artemis::data::ColorMap& cm) {
					ss->set(cm);
				});
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

