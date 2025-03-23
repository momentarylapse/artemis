//
// Created by michi on 09.03.25.
//

#include "GraphEditor.h"
#include <Session.h>
#include <graph/AutoConnect.h>
#include <graph/Graph.h>
#include <graph/Node.h>
#include <graph/NodeFactory.h>
#include <graph/Port.h>
#include <graph/Setting.h>
#include <lib/base/iter.h>
#include <lib/base/sort.h>
#include <lib/os/msg.h>
#include <lib/xhui/xhui.h>
#include <lib/xhui/Theme.h>
#include "DrawingHelper.h"


static constexpr float NODE_WIDTH = 150.0f;
static constexpr float NODE_HEIGHT = 50.0f;
static constexpr float PORT_DX = 30.0f;
static constexpr float PORT_DY = 10.0f;
static constexpr float PANEL_WIDTH = 320.0f;

class NodeListPanel : public xhui::Panel {
public:
	explicit NodeListPanel() : xhui::Panel("node-list-panel") {
		from_source(R"foodelim(
Dialog x ''
	Grid ? '' class=card
		Group node-list-group 'New node' height=350
			TabControl category 'field\\grid\\mesh\\render\\sim'
				ListView list-field 'class' nobar expandy dragsource=new-node
				ListView list-grid 'class' nobar expandy dragsource=new-node
				ListView list-mesh 'class' nobar expandy dragsource=new-node
				ListView list-renderer 'class' nobar expandy dragsource=new-node
				ListView list-simulation 'class' nobar expandy dragsource=new-node
)foodelim");

		struct X {
			graph::NodeCategory category;
			string name, title;
		};
		Array<X> categories = {
			{graph::NodeCategory::Field, "field", "field"},
			{graph::NodeCategory::Grid, "grid", "grid"},
			{graph::NodeCategory::Mesh, "mesh", "mesh"},
			{graph::NodeCategory::Renderer, "renderer", "render"},
			{graph::NodeCategory::Simulation, "simulation", "sim"}
		};
		for (const auto& [category, name, title]: categories) {
			string list_id = "list-" + name;
			const auto classes = graph::enumerate_nodes(category);
			for (const auto& c: classes) {
				add_string(list_id, c);
			}
			event_x(list_id, xhui::event_id::DragStart, [this, classes, list_id] {
				int i = get_int(list_id);
				if (i >= 0)
					get_window()->start_drag(classes[i], "add-node:" + classes[i]);
			});
		}

		size_mode_x = SizeMode::Shrink;
		size_mode_y = SizeMode::Shrink;
	}
};

GraphEditor::GraphEditor(Session* s) : obs::Node<Panel>("graph-editor") {
	session = s;
	graph = s->graph.get();

	from_source(R"foodelim(
Dialog x ''
	Overlay ? ''
		DrawingArea graph '' grabfocus width=400 expandx
		Grid overlay-grid '' margin=20
			Label ? '' expandy
			Label ? '' expandx
			---|
			Grid dock ''
			.
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
	event_x("graph", xhui::event_id::LeftButtonUp, [this] {
		on_left_button_up(get_window()->mouse_position());
	});
	event_x("graph", xhui::event_id::MouseLeave, [this] {
		on_mouse_leave(get_window()->mouse_position());
	});
	event_x("graph", xhui::event_id::KeyDown, [this] {
		on_key_down(get_window()->state.key_code);
	});

	event_x("graph", xhui::event_id::DragDrop, [this] {
		if (get_window()->drag.payload.match("add-node:*")) {
			graph->add_node(::graph::create_node(session, get_window()->drag.payload.sub(9)));
			graph->nodes.back()->pos = get_window()->mouse_position();
		}
	});

	graph->out_changed >> create_sink([this] {
		get_window()->request_redraw();
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

template<class P>
string port_description(P* p) {
	Array<string> flags;
	if (p->flags & graph::PortFlags::Mutable)
		flags.add("mutable");
	if (p->flags & graph::PortFlags::Optional)
		flags.add("optional");
	if (p->flags & graph::PortFlags::Multi)
		flags.add("multi");
	if (flags.num > 0)
		return format("<b>%s</b>, type <b>%s</b>  (%s)", p->name, p->class_->name, implode(flags, ", "));
	return format("<b>%s</b>, type <b>%s</b>", p->name, p->class_->name);
}

Array<vec2> GraphEditor::cable_spline(const graph::CableInfo& c) {
	vec2 A = node_out_port_pos(c.source, c.source_port);
	vec2 B = node_in_port_pos(c.sink, c.sink_port);
	return DrawingHelper::spline(A, A + vec2(0, 160), B - vec2(0, 160), B);
}


void GraphEditor::on_draw(Painter* p) {
	auto clip0 = p->clip();
	p->set_clip(_area);
	p->set_color(xhui::Theme::_default.background);
	p->draw_rect(_area);


	for (const auto& [i, c]: enumerate(graph->cables())) {
		p->set_color(Gray);
		p->set_line_width(3);
		if (selection and selection->type == HoverType::Cable and selection->index == i) {
			p->set_color(White);
			p->set_line_width(4);
		} else if (hover and hover->type == HoverType::Cable and hover->index == i) {
			p->set_color(color::interpolate(Gray, White, 0.3f));
			p->set_line_width(3);
		}
		p->draw_lines(cable_spline(c));
	}

	for (auto n: graph->nodes)
		draw_node(p, n);
	p->set_font("", xhui::Theme::_default.font_size, false, false);


	// new cable?
	if (get_window()->button(0)) {
		if (selection and selection->type == HoverType::OutPort) {
			p->set_color(White);
			p->set_line_width(3);
			p->draw_line(node_out_port_pos(selection->node, selection->index), get_window()->mouse_position());
		}
		if (selection and selection->type == HoverType::InPort) {
			p->set_color(White);
			p->set_line_width(3);
			p->draw_line(node_in_port_pos(selection->node, selection->index), get_window()->mouse_position());
		}
	}

	string tip;
	if (hover and hover->type == HoverType::OutPort)
		tip = format("output %s", port_description(hover->node->out_ports[hover->index]));
	if (hover and hover->type == HoverType::InPort)
		tip = format("input %s", port_description(hover->node->in_ports[hover->index]));
	if (hover and hover->type == HoverType::Cable) {
		const auto c = graph->cables()[hover->index];
		tip = format("cable, type <b>%s</b>", c.source->out_ports[c.source_port]->class_->name);
	}

	if (tip != "") {
		const auto l = TextLayout::from_format_string(tip);
		DrawingHelper::draw_text_layout_with_box(p, get_window()->mouse_position() + vec2(-10, 30), l);
	}

	p->set_clip(clip0);
}

void GraphEditor::draw_node(Painter* p, graph::Node* n) {
	p->set_font("", xhui::Theme::_default.font_size, true, false);

	if (selection and selection->type == HoverType::Node and selection->node == n) {
		p->set_color(Red.with_alpha(0.7f));
		p->set_roundness(14);
		p->draw_rect(node_area(n).grow(4));
	}
	color bg = color::interpolate(Orange, xhui::Theme::_default.background_low, 0.3f);
	if (n->flags & graph::NodeFlags::Resource)
		bg = color::interpolate(color(1, 0.3f, 0.4f, 1), xhui::Theme::_default.background_low, 0.3f);
	else if (n->flags & graph::NodeFlags::TimeDependent)
		bg = color::interpolate(color(1, 0, 0.7f, 0), xhui::Theme::_default.background_low, 0.3f);
	if (hover and hover->type == HoverType::Node and hover->node == n)
		bg = color::interpolate(bg, White, 0.2f);
	p->set_color(bg);
	p->set_roundness(10);
	p->draw_rect(node_area(n));

	p->set_color(White);
	float w = p->get_str_width(n->name);
	p->draw_str(n->pos + vec2(NODE_WIDTH / 2 - w/2, 5), n->name);
	if (n->dirty)
		p->draw_str(n->pos + vec2(NODE_WIDTH / 2, 35), "X");

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
}

bool spline_hover(const Array<vec2>& points, const vec2& m) {
	for (const vec2& p: points)
		if ((p - m).length() < 20)
			return true;
	return false;
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
	for (const auto& [i, c]: enumerate(graph->cables()))
		if (spline_hover(cable_spline(c), m))
			return Hover{HoverType::Cable, nullptr, i};
	return base::None;
}

void GraphEditor::on_mouse_move(const vec2& m, const vec2& d) {
	if (get_window()->button(0)) {
		if (selection and selection->type == HoverType::Node) {
			selection->node->pos = m - dnd_offset;
		} else if (selection and (selection->type == HoverType::OutPort or selection->type == HoverType::InPort)) {
			// new connection
			hover = get_hover(m);
		}
	} else {
		hover = get_hover(m);
	}

	request_redraw();
}

void GraphEditor::on_left_button_down(const vec2& m) {
	hover = get_hover(m);
	selection = hover;

	if (node_panel) {
		unembed(node_panel);
		node_panel = nullptr;
	}

	if (selection and selection->type == HoverType::Node) {
		node_panel = selection->node->create_panel();
		node_panel->min_width_user = PANEL_WIDTH;
		embed("dock", 0, 0, node_panel);

		dnd_offset = m - selection->node->pos;
	} else if (!selection) {
		node_panel = new NodeListPanel();
		node_panel->min_width_user = PANEL_WIDTH;
		embed("dock", 0, 0, node_panel);
	}

	request_redraw();
}

void GraphEditor::on_left_button_up(const vec2& m) {
	if (selection and selection->type == HoverType::OutPort) {
		if (hover and hover->type == HoverType::InPort) {
			try {
				::graph::auto_connect(graph, {selection->node, selection->index, hover->node, hover->index});
			} catch (Exception& e) {
				session->set_message(e.message());
			}
		}
	}
	if (selection and selection->type == HoverType::InPort) {
		if (hover and hover->type == HoverType::OutPort) {
			try {
				::graph::auto_connect(graph, {hover->node, hover->index, selection->node, selection->index});
			} catch (Exception& e) {
				session->set_message(e.message());
			}
		}
	}
}

void GraphEditor::on_mouse_leave(const vec2& m) {
	hover = base::None;
	request_redraw();
}

void GraphEditor::on_key_down(int key) {
	if (key == xhui::KEY_DELETE or key == xhui::KEY_BACKSPACE) {
		if (selection and selection->type == HoverType::Cable) {
			graph->unconnect(graph->cables()[selection->index]);
			selection = base::None;
			hover = base::None;
		}
		if (selection and selection->type == HoverType::Node) {
			graph->remove_node(selection->node);
			selection = base::None;
			hover = base::None;
		}
	}
}







