//
// Created by michi on 09.03.25.
//

#ifndef GRAPHEDITOR_H
#define GRAPHEDITOR_H


#include <lib/base/optional.h>
#include <lib/kaba/syntax/Node.h>
#include <lib/xhui/Panel.h>

class Session;

namespace graph {
	class Graph;
	class Node;
}

class GraphEditor : public xhui::Panel {
public:
	explicit GraphEditor(Session* s);

	void on_left_button_down(const vec2& m) override;
	void on_mouse_move(const vec2& m, const vec2& d) override;
	void on_draw(Painter* p);

	Session* session;
	graph::Graph* graph;

	enum class HoverType {
		Node,
		InPort,
		OutPort
	};

	struct Hover {
		HoverType type;
		graph::Node* node;
		int index;
	};
	base::optional<Hover> hover;
	base::optional<Hover> selection;
	base::optional<Hover> get_hover(const vec2& m);

	vec2 dnd_offset;
};



#endif //GRAPHEDITOR_H
