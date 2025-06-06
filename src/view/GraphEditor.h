//
// Created by michi on 09.03.25.
//

#ifndef GRAPHEDITOR_H
#define GRAPHEDITOR_H


#include <lib/base/optional.h>
#include <lib/pattern/Observable.h>
#include <lib/xhui/Panel.h>

class Session;

namespace dataflow {
	class Node;
	struct CableInfo;
}

namespace artemis::graph {
	class Graph;
}

class GraphEditor : public obs::Node<xhui::Panel> {
public:
	explicit GraphEditor(Session* s);

	void on_left_button_down(const vec2& m) override;
	void on_left_button_up(const vec2& m) override;
	void on_mouse_move(const vec2& m, const vec2& d) override;
	void on_mouse_leave(const vec2& m) override;
	void on_key_down(int key) override;
	void on_draw(Painter* p);

	void draw_node(Painter* p, dataflow::Node* node);

	static Array<vec2> cable_spline(const dataflow::CableInfo& c);

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

	vec2 dnd_offset;
};



#endif //GRAPHEDITOR_H
