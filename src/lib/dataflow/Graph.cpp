//
// Created by michi on 09.03.25.
//

#include "Graph.h"
#include "Port.h"
#include "Node.h"
#include <lib/base/algo.h>
#include <lib/base/error.h>
#include <lib/os/msg.h>
#include <lib/kaba/syntax/Class.h>
#include <lib/profiler/Profiler.h>

namespace dataflow {

Graph::Graph() : Node("graph") {
}

void Graph::clear() {
	auto _nodes = nodes;
	for (auto n: _nodes)
		remove_node(n);
}

void Graph::add_node(Node* node) {
	nodes.add(node);
	node->graph = this;
	profiler::set_parent(node->channel, channel);
	out_changed();
}

void Graph::remove_node(Node* node) {
	auto cc = cables();
	for (const auto& c: cc)
		if (c.source == node or c.sink == node)
			unconnect({c.source, c.source_port, c.sink, c.sink_port});
	base::remove(nodes, node);
	//delete node;
}

bool port_type_match(const OutPortBase& source, const InPortBase& sink) {
	if (!sink.type)
		return true;
	return source.type == sink.type;
}

base::expected<int> Graph::connect(OutPortBase& source, InPortBase& sink) {
	if (!port_type_match(source, sink))
		return base::Error{format("failed to connect: %s  vs  %s", source.type->name, sink.type->name)};
	if (sink.sources.num > 0 and !(sink.flags & PortFlags::Multi))
		return base::Error{format("failed to connect: sink already connected")};
	if ((sink.flags & PortFlags::Mutable) and !(source.flags & PortFlags::Mutable))
		return base::Error{format("failed to connect: sink is mutable, source is not")};

	sink.sources.add(&source);
	source.targets.add(&sink);
	out_changed();
	return 0;
}

base::expected<int> Graph::connect(const CableInfo& c) {
	return connect(*c.source->out_ports[c.source_port], *c.sink->in_ports[c.sink_port]);
}

void Graph::unconnect(OutPortBase& source, InPortBase& sink) {
	base::remove(sink.sources, &source);
	base::remove(source.targets, &sink);
	out_changed();
}

void Graph::unconnect(const CableInfo& c) {
	unconnect(*c.source->out_ports[c.source_port], *c.sink->in_ports[c.sink_port]);
}


Array<CableInfo> Graph::cables() const {
	Array<CableInfo> r;
	for (auto n: nodes)
		for (int i=0; i<n->in_ports.num; i++)
			for (auto source: n->in_ports[i]->sources)
				r.add({source->owner, source->port_index, n, i});
	return r;
}


bool Graph::iterate() {
	profiler::begin(channel);

	// TODO DAG
	bool updated_any = false;
	for (auto n: nodes)
		if (n->dirty and n->has_necessary_inputs()) {
			n->process();
			n->dirty = false;
			updated_any = true;
		}

	profiler::end(channel);
	return updated_any;
}

void Graph::reset_state() {
	for (auto n: nodes)
		n->dirty = true;
}





} // graph