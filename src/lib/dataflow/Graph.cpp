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

#if 0
#define db_out msg_write
#define db_right msg_right
#define db_left msg_left
#else
#define db_out(x)
#define db_right(x)
#define db_left(x)
#endif


namespace dataflow {

int CableInfo::source_port_no() const {
	return source->owner->out_ports.find(source);
}

int CableInfo::sink_port_no() const {
	return sink->owner->in_ports.find(sink);
}

Graph::Graph() : Node("graph") {
	_hidden_in_forwarder = new Node("in-forwarder");
	_hidden_out_forwarder = new Node("out-forwarder");
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
		if (c.source->owner == node or c.sink->owner == node)
			unconnect(c);
	base::remove(nodes, node);
	//delete node;
}

InPortBase* Graph::add_in_port_forward(InPortBase *target) {
	auto p = new InPortBase(this, format("%s:%s", target->owner->name, target->name), target->type, target->flags | PortFlags::Forwarding);
	auto pp = new OutPortBase(_hidden_in_forwarder.get(), ":fw:", target->type, nullptr, target->flags | PortFlags::Forwarding);
	p->link_partner = pp;
	pp->link_partner = p;
	_in_ports_forward.add(p);

	connect({pp, target});

	return p;
}

OutPortBase* Graph::add_out_port_forward(OutPortBase *target) {
	auto p = new OutPortBase(this, format("%s:%s", target->owner->name, target->name), target->type, target->generic_value_pointer, target->flags | PortFlags::Forwarding);
	auto pp = new InPortBase(_hidden_out_forwarder.get(), target->name, target->type, target->flags | PortFlags::Forwarding);
	p->link_partner = pp;
	pp->link_partner = p;
	_out_ports_forward.add(p);
	connect({target, pp});
	return p;
}


bool port_type_match(const OutPortBase& source, const InPortBase& sink) {
	if (!sink.type)
		return true;
	return source.type == sink.type;
}

base::expected<int> Graph::connect(const CableInfo& c) {
	if (!port_type_match(*c.source, *c.sink))
		return base::Error{format("failed to connect: %s  vs  %s", c.source->type->name, c.sink->type->name)};
	if (c.sink->sources.num > 0 and !(c.sink->flags & PortFlags::Multi))
		return base::Error{format("failed to connect: sink already connected")};
	if ((c.sink->flags & PortFlags::Mutable) and !(c.source->flags & PortFlags::Mutable))
		return base::Error{format("failed to connect: sink is mutable, source is not")};

	c.sink->sources.add(c.source);
	c.source->targets.add(c.sink);
	c.sink->owner->on_input_changed(c.sink);
	out_changed();
	return 0;
}

base::expected<int> Graph::connect(Node* source, int source_port, Node* sink, int sink_port) {
	return connect({source->out_ports[source_port], sink->in_ports[sink_port]});
}

void Graph::unconnect(OutPortBase& source, InPortBase& sink) {
	base::remove(sink.sources, &source);
	base::remove(source.targets, &sink);
	sink.owner->on_input_changed(&sink);
	out_changed();
}

void Graph::unconnect(const CableInfo& c) {
	unconnect(*c.source, *c.sink);
}


Array<CableInfo> Graph::cables() const {
	Array<CableInfo> r;
	for (auto n: nodes)
		for (auto sink: n->in_ports)
			for (auto source: sink->sources)
				r.add({source, sink});
	return r;
}

static string state2str(NodeState s) {
	switch (s) {
	case NodeState::Dirty:
		return "dirty";
	case NodeState::Uninitialized:
		return "uninit";
	case NodeState::Complete:
		return "complete";
	}
	return "?";
}


bool Graph::iterate() {
	profiler::begin(channel);
	db_out("<<<iter " + name);
	db_right();

	for (auto n: nodes) {
		db_out(format("%s   %s  input=%d", n->name, state2str(n->state), (int)n->has_necessary_inputs()));
		for (auto p: n->in_ports)
			db_out(format("   %s  %d", p->name, p->generic_values().num));
	}

	// TODO DAG
	bool updated_any = false;
	for (auto n: nodes)
		if (n->state == NodeState::Uninitialized) {
			db_out("  INIT " + n->name);
			n->additional_init();
			updated_any = true;
		}

	for (auto n: nodes) {
		if (n->flags & NodeFlags::Meta) {
			updated_any |= static_cast<Graph*>(n)->iterate();
		} else if (n->state == NodeState::Dirty and n->has_necessary_inputs()) {
			db_out("  PROC " + n->name);
			n->process();
			n->state = NodeState::Complete;
			updated_any = true;
		}
	}

	if (!updated_any)
		state = NodeState::Complete;


	profiler::end(channel);
	db_left();
	db_out("iter>>>");
	return updated_any;
}

void Graph::on_process() {
	iterate();
}

void Graph::reset_state() {
	for (auto n: nodes)
		if (n->state == NodeState::Complete)
			n->state = NodeState::Dirty;
}

	Graph* Graph::group_nodes(const base::set<Node*>& selected_nodes) {
	auto _cables = cables();

	auto group = new Graph();
	group->flags = dataflow::NodeFlags::Meta;
	group->name = "Group";
	group->state = dataflow::NodeState::Dirty;
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
		if (selected_nodes.contains(c.source->owner) and !selected_nodes.contains(c.sink->owner)) {
			// group output port
			unconnect(c);
			group->add_out_port_forward(c.source);
			connect(CableInfo{group->out_ports.back(), c.sink});
		} else if (!selected_nodes.contains(c.source->owner) and selected_nodes.contains(c.sink->owner)) {
			// group input port
			unconnect(c);
			group->add_in_port_forward(c.sink);
			connect(CableInfo{c.source, group->in_ports.back()});
		}
	}

	return group;
}


} // graph