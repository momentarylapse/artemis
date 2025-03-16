//
// Created by michi on 09.03.25.
//

#include "Graph.h"

#include <lib/base/algo.h>

#include "Port.h"
#include "Node.h"
#include <lib/os/msg.h>

namespace graph {

Graph::Graph(Session* s) {
	session = s;
}

void Graph::clear() {
	auto _nodes = nodes;
	for (auto n: _nodes)
		remove_node(n);
}

void Graph::add_node(graph::Node* node) {
	nodes.add(node);
	out_changed();
}

void Graph::remove_node(graph::Node* node) {
	auto cc = cables();
	for (const auto& c: cc)
		if (c.source == node or c.sink == node)
			unconnect(c.source, c.source_port, c.sink, c.sink_port);
	base::remove(nodes, node);
	//delete node;
}


bool Graph::connect(OutPortBase& source, InPortBase& sink) {
	if (sink.class_ != source.class_) {
		msg_error(format("failed to connect: %s  vs  %s", source.class_->name, sink.class_->name));
		return false;
	}
	if (sink.source) {
		msg_error(format("failed to connect: sink already connected"));
		return false;
	}
	if ((sink.flags & PortFlags::Mutable) and !(source.flags & PortFlags::Mutable)) {
		msg_error(format("failed to connect: sink is mutable, source is not"));
		return false;
	}
	sink.source = &source;
	source.targets.add(&sink);
	out_changed();
	return true;
}

bool Graph::connect(graph::Node* source, int source_port, graph::Node* sink, int sink_port) {
	return connect(*source->out_ports[source_port], *sink->in_ports[sink_port]);
}

void Graph::unconnect(OutPortBase& source, InPortBase& sink) {
	sink.source = nullptr;
	base::remove(source.targets, &sink);
	out_changed();
}

void Graph::unconnect(graph::Node* source, int source_port, graph::Node* sink, int sink_port) {
	unconnect(*source->out_ports[source_port], *sink->in_ports[sink_port]);
}


Array<CableInfo> Graph::cables() const {
	Array<CableInfo> r;
	for (auto n: nodes)
		for (int i=0; i<n->in_ports.num; i++)
			if (n->in_ports[i]->source)
				r.add({n->in_ports[i]->source->owner, n->in_ports[i]->source->port_index, n, i});
	return r;
}


void Graph::iterate() {
	// TODO DAG
	for (auto n: nodes)
		if (n->dirty)
			n->process();
}




} // graph