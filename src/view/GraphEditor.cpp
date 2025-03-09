//
// Created by michi on 09.03.25.
//

#include "GraphEditor.h"
#include <Session.h>
#include <graph/Graph.h>
#include <graph/Node.h>
#include <graph/Port.h>
#include <lib/os/msg.h>
#include <lib/xhui/xhui.h>
#include <lib/xhui/Theme.h>

static constexpr float NODE_WIDTH = 150.0f;
static constexpr float NODE_HEIGHT = 50.0f;
static constexpr float PORT_DX = 50.0f;
static constexpr float PORT_DY = 10.0f;

GraphEditor::GraphEditor(Session* s) : Panel("graph-editor") {
	session = s;
	graph = s->graph.get();

	from_source(R"foodelim(
Dialog x ''
	DrawingArea graph '' grabfocus
)foodelim");


	event_xp("graph", xhui::event_id::Draw, [this] (Painter* p) {
		on_draw(p);
	});
	event_x("graph", xhui::event_id::MouseMove, [this] {
		on_mouse_move(get_window()->mouse_position(), {0,0});
	});
	event_x("graph", xhui::event_id::LeftButtonDown, [this] {
		on_left_button_down(get_window()->mouse_position());
	});
}

rect node_area(graph::Node* n) {
	return {n->pos, n->pos + vec2(NODE_WIDTH, NODE_HEIGHT)};
}

vec2 node_in_port_pos(graph::Node* n, int i) {
	return n->pos + vec2(NODE_WIDTH / 2 + ((float)i - (float)(n->in_ports.num - 1) / 2) * PORT_DX, -PORT_DY);
}

vec2 node_out_port_pos(graph::Node* n, int i) {
	return n->pos + vec2(NODE_WIDTH / 2 + ((float)i - (float)(n->out_ports.num - 1) / 2) * PORT_DX, NODE_HEIGHT + PORT_DY);
}

void GraphEditor::on_draw(Painter* p) {
	float w = (float)p->width;
	float h = (float)p->height;

	p->set_color(xhui::Theme::_default.background_low);
	p->draw_rect({0,w, 0,h});

	for (auto n: graph->nodes) {
		color bg = Orange.with_alpha(0.5f);
		if (selection and selection->type == HoverType::Node and selection->node == n)
			bg = Red.with_alpha(0.7f);
		if (hover and hover->type == HoverType::Node and hover->node == n)
			bg = color::interpolate(bg, White, 0.3f);
		p->set_color(bg);
		p->set_roundness(10);
		p->draw_rect(node_area(n));

		p->set_color(White);
		p->draw_str(n->pos + vec2(20, 5), n->name);

		for (int i=0; i<n->in_ports.num; i++) {
			p->set_color(Gray);
			if (hover and hover->type == HoverType::InPort and hover->node == n and hover->index == i)
				p->set_color(White);
			p->draw_circle(node_in_port_pos(n, i), 5);
		}
		for (int i=0; i<n->out_ports.num; i++) {
			p->set_color(Gray);
			if (hover and hover->type == HoverType::OutPort and hover->node == n and hover->index == i)
				p->set_color(White);
			p->draw_circle(node_out_port_pos(n, i), 5);
		}


		for (int i=0; i<n->in_ports.num; i++)
			if (n->in_ports[i]->source) {
				p->set_color(Gray);
				p->set_line_width(3);
				p->draw_line(node_out_port_pos(n->in_ports[i]->source->owner, 0), node_in_port_pos(n, i));
			}
	}
}

base::optional<GraphEditor::Hover> GraphEditor::get_hover(const vec2& m) {
	for (auto n: graph->nodes) {
		for (int i=0; i<n->in_ports.num; i++)
			if ((m - node_in_port_pos(n, i)).length() < 15)
				return Hover{HoverType::InPort, n, i};
		for (int i=0; i<n->out_ports.num; i++)
			if ((m - node_out_port_pos(n, i)).length() < 15)
				return Hover{HoverType::OutPort, n, i};
		if (node_area(n).inside(m))
			return Hover{HoverType::Node, n, -1};
	}
	return base::None;
}

void GraphEditor::on_mouse_move(const vec2& m, const vec2& d) {
	hover = get_hover(m);

	if (get_window()->button(0) and selection and selection->type == HoverType::Node) {
		selection->node->pos = m - dnd_offset;
	}

	request_redraw();
}

void GraphEditor::on_left_button_down(const vec2& m) {
	hover = get_hover(m);
	selection = hover;

	if (selection and selection->type == HoverType::Node) {
		dnd_offset = m - selection->node->pos;
	}

	request_redraw();
}





