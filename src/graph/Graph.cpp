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

void Graph::connect(OutPortBase& source, InPortBase& sink) {
	if (sink.class_ != source.class_) {
		msg_error(format("failed to connect: %s  vs  %s", source.class_->name, sink.class_->name));
		return;
	}
	if (sink.source) {
		msg_error(format("failed to connect: sink already connected"));
		return;
	}
	sink.source = &source;
	source.targets.add(&sink);
}

void Graph::connect(Node* source, int source_port, Node* sink, int sink_port) {
	connect(*source->out_ports[source_port], *sink->in_ports[sink_port]);
}

void Graph::iterate() {
	// TODO DAG
	for (auto n: nodes)
		if (n->dirty)
			n->process();
}




} // graph