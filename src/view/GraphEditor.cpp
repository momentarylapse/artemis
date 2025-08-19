//
// Created by michi on 09.03.25.
//

#include "GraphEditor.h"
#include <Session.h>
#include <graph/AutoConnect.h>
#include <graph/Graph.h>
#include <lib/dataflow/Node.h>
#include <graph/NodeFactory.h>
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>
#include <lib/dataflow/Type.h>
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
static constexpr float PANEL_WIDTH = 280.0f;

class NodeListPanel : public xhui::Panel {
public:
	Array<string> classes_filtered;

	explicit NodeListPanel() : xhui::Panel("node-list-panel") {
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
};

GraphEditor::GraphEditor(Session* s) : obs::Node<Panel>("graph-editor") {
	session = s;
	graph = s->graph;

	from_source(R"foodelim(
Dialog x ''
	Overlay ? ''
		DrawingArea graph '' grabfocus width=400 expandx
		Grid overlay-grid '' margin=25
			Grid ? '' spacing=20
				Label ? '' ignorehover expandx
				Button add-node '+' height=50 width=50 padding=7 noexpandx ignorefocus
				Button show-code-editor 'E' height=50 width=50 padding=7 noexpandx ignorefocus
			---|
			Grid ? ''
				Label ? '' ignorehover expandy
				Label ? '' ignorehover expandx
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
	event_x("graph", xhui::event_id::MouseWheel, [this] {
		on_mouse_wheel(get_window()->state.scroll);
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
			const auto m = get_window()->mouse_position();
			auto n = artemis::graph::create_node(session, get_window()->drag.payload.sub(9));
			n->pos = from_screen(m);
			graph->add_node(n);
			if (hover and hover->type == HoverType::OutPort and n->in_ports.num > 0) {
				if (dataflow::port_type_match(*hover->node->out_ports[hover->index], *n->in_ports[0])) {
					n->pos.y += 50;
					graph->connect(dataflow::CableInfo{hover->node, hover->index, n, 0});
				}
			} else if (hover and hover->type == HoverType::InPort and n->out_ports.num > 0) {
				if (dataflow::port_type_match(*n->out_ports[0], *hover->node->in_ports[hover->index])) {
					n->pos.y -= 100;
					graph->connect(dataflow::CableInfo{n, 0, hover->node, hover->index});
				}
			}
		}
	});
	event("add-node", [this] {
		open_node_list_panel();
	});
	event("show-code-editor", [this] {
		if (auto w = get_window())
			w->set_visible("code-editor-panel", true);
	});

	graph->out_changed >> create_sink([this] {
		get_window()->request_redraw();
	});
}

rect GraphEditor::node_area(dataflow::Node* n) {
	return {to_screen(n->pos), to_screen(n->pos + vec2(NODE_WIDTH, NODE_HEIGHT))};
}

vec2 GraphEditor::node_in_port_pos(dataflow::Node* n, int i) {
	return to_screen(n->pos + vec2(NODE_WIDTH / 2 + ((float)i - (float)(n->in_ports.num - 1) / 2) * PORT_DX, -PORT_DY));
}

vec2 GraphEditor::node_out_port_pos(dataflow::Node* n, int i) {
	return to_screen(n->pos + vec2(NODE_WIDTH / 2 + ((float)i - (float)(n->out_ports.num - 1) / 2) * PORT_DX, NODE_HEIGHT + PORT_DY));
}

template<class P>
string port_description(P* p) {
	Array<string> flags;
	if (p->flags & dataflow::PortFlags::Mutable)
		flags.add("mutable");
	if (p->flags & dataflow::PortFlags::Optional)
		flags.add("optional");
	if (p->flags & dataflow::PortFlags::Multi)
		flags.add("multi");
	string type = p->class_->name;
	if (flags.num > 0)
		return format("<b>%s</b>, type <b>%s</b>  (%s)", p->name, type, implode(flags, ", "));
	return format("<b>%s</b>, type <b>%s</b>", p->name, type);
}

vec2 GraphEditor::to_screen(const vec2 &p) const {
	return p * view_scale + view_offset;
}

vec2 GraphEditor::from_screen(const vec2 &p) const {
	return (p - view_offset) / view_scale;
}


Array<vec2> GraphEditor::cable_spline(const dataflow::CableInfo& c) {
	vec2 A = node_out_port_pos(c.source, c.source_port);
	vec2 B = node_in_port_pos(c.sink, c.sink_port);
	return DrawingHelper::spline(A, A + vec2(0, 160*view_scale), B - vec2(0, 160*view_scale), B);
}

void GraphEditor::draw_grid(Painter* p) {
	p->set_color(xhui::Theme::_default.background);
	p->draw_rect(_area);
	p->set_line_width(max(1.0f * view_scale, 1.0f));
	p->set_contiguous(false);

	rect view_area = rect(from_screen(_area.p00()), from_screen(_area.p11()));
	color c0 = color::interpolate(xhui::Theme::_default.background, xhui::Theme::_default.text_disabled, 0.07f * min(view_scale, 1.0f));
	color c1 = color::interpolate(xhui::Theme::_default.background, xhui::Theme::_default.text_disabled, 0.15f * min(view_scale, 1.0f));
	const float D = 20.0f;
	Array<vec2> points;
	for (int i=(int)floorf(view_area.y1 / D); i<(int)ceilf(view_area.y2 / D); i++) {
		if (i % 5 == 0)
			continue;
		float y = to_screen({0, (float)i*D}).y;
		points.add({_area.x1, y});
		points.add({_area.x2, y});
	}
	for (int i=(int)floorf(view_area.x1 / D); i<(int)ceilf(view_area.x2 / D); i++) {
		if (i % 5 == 0)
			continue;
		float x = to_screen({(float)i*D, 0}).x;
		points.add({x, _area.y1});
		points.add({x, _area.y2});
	}
	p->set_color(c0);
	p->draw_lines(points);


	points.clear();
	for (int i=(int)floorf(view_area.y1 / (D*5)); i<(int)ceilf(view_area.y2 / (D*5)); i++) {
		float y = to_screen({0, (float)i*D*5}).y;
		points.add({_area.x1, y});
		points.add({_area.x2, y});
	}
	for (int i=(int)floorf(view_area.x1 / (D*5)); i<(int)ceilf(view_area.x2 / (D*5)); i++) {
		float x = to_screen({(float)i*D*5, 0}).x;
		points.add({x, _area.y1});
		points.add({x, _area.y2});
	}
	p->set_color(c1);
	p->draw_lines(points);
	p->set_contiguous(true);
}

void GraphEditor::on_draw(Painter* p) {
	auto clip0 = p->clip();
	p->set_clip(_area);

	draw_grid(p);

	for (const auto& [i, c]: enumerate(graph->cables())) {
		p->set_color(Gray);
		p->set_line_width(3 * view_scale);
		if (selection and selection->type == HoverType::Cable and selection->index == i) {
			p->set_color(White);
			p->set_line_width(4 * view_scale);
		} else if (hover and hover->type == HoverType::Cable and hover->index == i) {
			p->set_color(color::interpolate(Gray, White, 0.3f));
			p->set_line_width(3 * view_scale);
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
			p->set_line_width(3 * view_scale);
			p->draw_line(node_out_port_pos(selection->node, selection->index), get_window()->mouse_position());
		}
		if (selection and selection->type == HoverType::InPort) {
			p->set_color(White);
			p->set_line_width(3 * view_scale);
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

void GraphEditor::draw_node(Painter* p, dataflow::Node* n) {
	p->set_font("", xhui::Theme::_default.font_size * view_scale, true, false);

	if (selection and selection->type == HoverType::Node and selection->node == n) {
		p->set_color(Red.with_alpha(0.7f));
		p->set_roundness(14 * view_scale);
		p->draw_rect(node_area(n).grow(4 * view_scale));
	}
	color bg = color::interpolate(Orange, xhui::Theme::_default.background_low, 0.3f);
	if (n->flags & dataflow::NodeFlags::Resource)
		bg = color::interpolate(color(1, 0.3f, 0.4f, 1), xhui::Theme::_default.background_low, 0.3f);
	else if (n->flags & dataflow::NodeFlags::TimeDependent)
		bg = color::interpolate(color(1, 0, 0.7f, 0), xhui::Theme::_default.background_low, 0.3f);
	if (hover and hover->type == HoverType::Node and hover->node == n)
		bg = color::interpolate(bg, White, 0.2f);
	p->set_color(bg);
	p->set_roundness(10 * view_scale);
	p->draw_rect(node_area(n));

	p->set_color(White);
	p->set_font("", xhui::Theme::_default.font_size * view_scale, true, false);
	{
		const string name = n->name.explode(":").back();
		const float w = p->get_str_width(name) / view_scale;
		p->draw_str(to_screen(n->pos + vec2(NODE_WIDTH / 2 - w/2, 5)), name);
		if (n->dirty)
			p->draw_str(to_screen(n->pos + vec2(NODE_WIDTH / 2, 35)), "X");
	}
	p->set_font("", xhui::Theme::_default.font_size * view_scale, false, false);

	if (n->name.find(":") >= 0) {
		p->set_color(White.with_alpha(0.7f));
		const string class_name = n->name.explode(":")[0];
		const float w = p->get_str_width(class_name) / view_scale;
		p->draw_str(to_screen(n->pos + vec2(NODE_WIDTH / 2 - w/2, 20)), class_name);
	}


	for (int i=0; i<n->in_ports.num; i++) {
		p->set_color(Gray);
		if (hover and hover->type == HoverType::InPort and hover->node == n and hover->index == i)
			p->set_color(White);
		p->draw_circle(node_in_port_pos(n, i), 5 * view_scale);
	}
	for (int i=0; i<n->out_ports.num; i++) {
		p->set_color(Gray);
		if (hover and hover->type == HoverType::OutPort and hover->node == n and hover->index == i)
			p->set_color(White);
		p->draw_circle(node_out_port_pos(n, i), 5 * view_scale);
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
		if (get_window()->drag.active and get_window()->drag.payload.match("add-node:*")) {
			hover = get_hover(m);
		} else if (selection and selection->type == HoverType::Node) {
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

void GraphEditor::on_mouse_wheel(const vec2 &d) {
	const vec2 m = get_window()->state.m;
	const vec2 mm = from_screen(m);
	view_scale = clamp(view_scale * expf(0.2f * d.y), 0.5f, 10.0f);
	view_offset -= to_screen(mm) - m;

	hover = get_hover(m);
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
	}

	request_redraw();
}

void GraphEditor::on_left_button_up(const vec2& m) {
	if (selection and selection->type == HoverType::OutPort) {
		if (hover and hover->type == HoverType::InPort) {
			auto r = artemis::graph::auto_connect(graph, {selection->node, selection->index, hover->node, hover->index});
			if (!r)
				session->set_message(r.error().msg);
		}
	}
	if (selection and selection->type == HoverType::InPort) {
		if (hover and hover->type == HoverType::OutPort) {
			auto r = artemis::graph::auto_connect(graph, {hover->node, hover->index, selection->node, selection->index});
			if (!r)
				session->set_message(r.error().msg);
		}
	}
}

void GraphEditor::on_mouse_leave(const vec2& m) {
	hover = base::None;
	request_redraw();
}

void GraphEditor::open_node_list_panel() {
	node_panel = new NodeListPanel();
	node_panel->min_width_user = PANEL_WIDTH;
	embed("dock", 0, 0, node_panel);
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
	if (key == xhui::KEY_TAB)
		open_node_list_panel();
}







