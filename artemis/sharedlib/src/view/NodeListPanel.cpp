//
// Created by michi on 8/22/25.
//

#include "NodeListPanel.h"
#include <lib/dataflow/Node.h>
#include <lib/xhui/xhui.h>
#include <graph/NodeFactory.h>

namespace artemis::view {

NodeListPanel::NodeListPanel() : xhui::Panel("node-list-panel") {
	from_source(R"foodelim(
Dialog x ''
	Grid ? '' class=card
		Group node-list-group 'New node' height=500
			Grid ? ''
				Edit filter ''
				---|
				ListView list 'class' nobar expandy dragsource=new-node format=m
)foodelim");

	struct X {
		dataflow::NodeCategory category;
		string name, title;
	};
	Array<X> categories = {
		{dataflow::NodeCategory::Field, "field", "field"},
		{dataflow::NodeCategory::Grid, "grid", "grid"},
		{dataflow::NodeCategory::Mesh, "mesh", "mesh"},
		{dataflow::NodeCategory::Renderer, "renderer", "render"},
		{dataflow::NodeCategory::Simulation, "simulation", "sim"}
	};
	string list_id = "list";
	const auto all_classes = artemis::graph::enumerate_nodes(dataflow::NodeCategory::None);
	classes_filtered = all_classes;
	auto fill_list = [this, list_id] {
		reset(list_id);
		for (const auto& c: classes_filtered)
			add_string(list_id, c);// + "\n<b> x</b>");
	};
	fill_list();

	xhui::run_later(0.1f, [this] {
		activate("filter");
	});

	event("filter", [this, all_classes, fill_list] {
		string filter = get_string("filter").lower();
		classes_filtered.clear();
		for (const auto& c: all_classes)
			if (c.lower().find(filter) >= 0)
				classes_filtered.add(c);
		fill_list();
	});
	event_x("list", xhui::event_id::DragStart, [this, list_id] {
		int i = get_int(list_id);
		if (i >= 0)
			get_window()->start_drag(classes_filtered[i], "add-node:" + classes_filtered[i]);
	});

	size_mode_x = SizeMode::Shrink;
	size_mode_y = SizeMode::Shrink;
}

}
