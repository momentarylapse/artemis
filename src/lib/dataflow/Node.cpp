//
// Created by michi on 09.03.25.
//

#include "Node.h"
#include "Setting.h"
#include "Port.h"
#include <view/DefaultNodePanel.h>
#include <lib/any/any.h>
#include <lib/os/msg.h>
#include <lib/profiler/Profiler.h>

namespace dataflow {

NodeFlags operator|(NodeFlags a, NodeFlags b) {
	return (NodeFlags)((int)a | (int)b);
}

bool operator&(NodeFlags a, NodeFlags b) {
	return (int)a & (int)b;
}

Node::Node(const string& name) {
	this->name = name;
	pos = vec2(0, 0);
	channel = profiler::create_channel(name);
	state = NodeState::Dirty;
}

Node::~Node() {
	profiler::delete_channel(channel);
}

void Node::process() {
	profiler::begin(channel);
	on_process();
	profiler::end(channel);
}

void Node::on_settings_changed(SettingBase*) {
	if (state == NodeState::Complete)
		state = NodeState::Dirty;
}

void Node::on_input_changed(InPortBase* port) {
	//msg_write("  <" + name + "  INPUT CHANGED>");
	if (state == NodeState::Complete)
		state = NodeState::Dirty;
	if (port->link_partner)
		port->link_partner->mutated();
}

void Node::set(const string& key, const Any& value) {
	for (auto& s: settings)
		if (s->name == key) {
			s->set_generic(value);
			return;
		}

	msg_error(format("unknown setting '%s' of Node '%s'", key, name));
}

Any Node::get(const string& key) const {
	for (auto& s: settings)
		if (s->name == key)
			return s->get_generic();

	msg_error(format("unknown setting '%s' of Node '%s'", key, name));
	return {};
}


bool Node::has_necessary_inputs() const {
	if (state == NodeState::Uninitialized)
		return false;
	for (auto p: in_ports)
		if (!(p->flags & PortFlags::Optional) and !p->has_value())
			return false;
	return true;
}


xhui::Panel* Node::create_panel() {
	return new DefaultNodePanel(this);
}

}