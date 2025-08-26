//
// Created by michi on 09.03.25.
//

#pragma once


#include <lib/base/optional.h>
#include <lib/pattern/Observable.h>
#include <lib/xhui/Panel.h>

#include "lib/base/set.h"

class Session;

namespace dataflow {
	class Node;
	struct CableInfo;
}

namespace artemis::graph {
	class Graph;
}

namespace artemis::view {

class GraphEditor : public obs::Node<xhui::Panel> {
public:
	explicit GraphEditor(Session* s);

	void on_left_button_down(const vec2& m) override;
	void on_left_button_up(const vec2& m) override;
	void on_mouse_move(const vec2& m, const vec2& d) override;
	void on_mouse_wheel(const vec2 &d) override;
	void on_mouse_leave(const vec2& m) override;
	void on_key_down(int key) override;
	void on_draw(Painter* p);

	void open_node_list_panel();
	void open_node_panel(dataflow::Node* n);

	void draw_grid(Painter* p);
	void draw_node(Painter* p, dataflow::Node* node);

	rect node_area(dataflow::Node* n);
	vec2 node_in_port_pos(dataflow::Node* n, int i);
	vec2 node_out_port_pos(dataflow::Node* n, int i);
	Array<vec2> cable_spline(const dataflow::CableInfo& c);

	Session* session;
	artemis::graph::Graph* graph;
	xhui::Panel* node_panel = nullptr;

	enum class HoverType {
		Node,
		InPort,
		OutPort,
		Cable
	};

	struct Hover {
		HoverType type;
		dataflow::Node* node;
		int index;
	};
	base::optional<Hover> hover;
	base::optional<Hover> selection;
	base::optional<Hover> get_hover(const vec2& m);
	base::set<dataflow::Node*> selected_nodes;

	enum class Mode {
		Default,
		CreatingNewCable,
		Selecting,
		MovingNodes
	} mode = Mode::Default;
	vec2 selection_start = vec2(0,0);

	vec2 moving_last_pos = vec2(0,0);
	float view_scale = 1.0f;
	vec2 view_offset = vec2(0,0);
	vec2 to_screen(const vec2& p) const;
	vec2 from_screen(const vec2& p) const;
};

}
