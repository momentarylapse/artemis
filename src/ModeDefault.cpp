//
// Created by michi on 09.03.25.
//

#include "ModeDefault.h"

#include <Session.h>
#include <graph/Graph.h>
#include <graph/Node.h>
#include <graph/NodeFactory.h>
#include <graph/Port.h>
#include <lib/image/Painter.h>
#include <lib/xhui/Theme.h>
#include <view/ArtemisWindow.h>
#include <view/MultiView.h>

static constexpr float NODE_WIDTH = 150.0f;
static constexpr float NODE_HEIGHT = 50.0f;
static constexpr float PORT_DX = 50.0f;
static constexpr float PORT_DY = 10.0f;


ModeDefault::ModeDefault(Session* s) : Mode(s) {
	multi_view = new MultiView(session);
	//data = new DataModel(session);
	//generic_data = data.get();

	graph = new graph::Graph(session);
	graph->add_node(graph::create_node("TeapotMesh"));
	graph->add_node(graph::create_node("MeshRenderer"));
	graph->connect(graph->nodes[0], 0, graph->nodes[1], 0);

	graph->nodes[0]->pos = {200, 100};
	graph->nodes[1]->pos = {200, 500};

	session->win->event_xp("graph", xhui::event_id::Draw, [this] (Painter* p) {
		draw_graph(p);
	});
}

void ModeDefault::on_draw_win(const RenderParams& params, MultiViewWindow* win) {

}

void ModeDefault::on_draw_post(Painter* p) {
}

vec2 node_in_port_pos(graph::Node* n, int i) {
	return n->pos + vec2(NODE_WIDTH / 2 + ((float)i - (float)(n->in_ports.num - 1) / 2) * PORT_DX, -PORT_DY);
}

vec2 node_out_port_pos(graph::Node* n, int i) {
	return n->pos + vec2(NODE_WIDTH / 2 + ((float)i - (float)(n->out_ports.num - 1) / 2) * PORT_DX, NODE_HEIGHT + PORT_DY);
}

void ModeDefault::draw_graph(Painter* p) {
	float w = (float)p->width;
	float h = (float)p->height;

	p->set_color(xhui::Theme::_default.background_low);
	p->draw_rect({0,w, 0,h});

	for (auto n: graph->nodes) {
		p->set_color(Red.with_alpha(0.3f));
		p->set_roundness(10);
		p->draw_rect({n->pos, n->pos + vec2(NODE_WIDTH, NODE_HEIGHT)});

		p->set_color(Red);
		p->draw_str(n->pos + vec2(20, 5), n->name);

		for (int i=0; i<n->in_ports.num; i++) {
			p->set_color(Red);
			p->draw_circle(node_in_port_pos(n, i), 5);
		}
		for (int i=0; i<n->out_ports.num; i++) {
			p->set_color(Red);
			p->draw_circle(node_out_port_pos(n, i), 5);
		}


		for (int i=0; i<n->in_ports.num; i++)
			if (n->in_ports[i]->source) {
				p->set_color(Red);
				p->draw_line(node_out_port_pos(n->in_ports[i]->source->owner, 0), node_in_port_pos(n, i));
			}
	}
}



