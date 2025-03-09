//
// Created by michi on 09.03.25.
//

#include "Graph.h"
#include "Port.h"
#include "Node.h"
#include <lib/os/msg.h>

namespace graph {

Graph::Graph(Session* s) {
	session = s;
}


void Graph::add_node(Node* node) {
	nodes.add(node);
}

void Graph::connect(OutPortBase& out, InPortBase& in) {
	if (in.class_ != out.class_) {
		msg_error(format("failed to connect: %s  vs  %s", out.class_->name, in.class_->name));
		return;
	}
	in.source = &out;
}

void Graph::connect(Node* source, int source_port, Node* sink, int sink_port) {
	connect(*source->out_ports[source_port], *sink->in_ports[sink_port]);
}

void Graph::iterate() {
	// TODO DAG
	// TODO state/only when needed
	for (auto n: nodes)
		n->process();
}




} // graph