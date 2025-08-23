//
// Created by Michael Ankele on 2025-03-31.
//

#include "Graph.h"
#include <lib/dataflow/Node.h>

#include "lib/os/msg.h"

namespace artemis::graph {

Graph::Graph(Session* s) {
	session = s;
	t = 0;
	dt = 0.1f;
}


void Graph::iterate_simulation() {
	t += dt;
	for (auto n: nodes)
		if (n->flags & dataflow::NodeFlags::TimeDependent and n->has_necessary_inputs()) {
			n->process();
			n->dirty = false;
		}
}

Graph* Graph::group_nodes(const base::set<Node*>& selected_nodes) {
	auto _cables = cables();

	auto group = new Graph(session);
	group->flags = dataflow::NodeFlags::Meta;
	group->name = "Group";
	add_node(group);

	// center
	group->pos = vec2(0,0);
	for (auto n: selected_nodes)
		group->pos += n->pos;
	group->pos /= (float)selected_nodes.num;

	for (auto n: selected_nodes) {
	//	remove_node(n); // would also unconnect...
		n->pos -= group->pos;
		nodes.erase(nodes.find(n));
		group->add_node(n);
	}

	// group ports
	for (const auto& c: _cables) {
		if (selected_nodes.contains(c.source) and !selected_nodes.contains(c.sink)) {
			// group output port
			unconnect(c);
			group->add_out_port_forward(c.source->out_ports[c.source_port]);
			connect(dataflow::CableInfo{group, group->out_ports.num - 1, c.sink, c.sink_port});
		} else if (!selected_nodes.contains(c.source) and selected_nodes.contains(c.sink)) {
			// group input port
			unconnect(c);
			group->add_in_port_forward(c.sink->in_ports[c.sink_port]);
			connect(dataflow::CableInfo{c.source, c.source_port, group, group->in_ports.num - 1});
		}
	}

	return group;
}


DataGraph::DataGraph(Session* session) : Data(session, -1) {
	graph = new Graph(session);
}

void DataGraph::reset() {
	graph->clear();
}

}
