//
// Created by michi on 09.03.25.
//

#include "Node.h"
#include "Setting.h"
#include <view/DefaultNodePanel.h>
#include <lib/os/msg.h>

#include "Port.h"

namespace graph {

NodeFlags operator|(NodeFlags a, NodeFlags b) {
	return (NodeFlags)((int)a | (int)b);
}

bool operator&(NodeFlags a, NodeFlags b) {
	return (int)a & (int)b;
}

Node::Node(const string& name) {
	this->name = name;
	pos = vec2(0, 0);
}

void Node::set(const string& key, const Any& value) {;
	for (auto& s: settings)
		if (s->name == key) {
			s->set_generic(value);
			return;
		}

	msg_error(format("unknown setting '%s' of Node '%s'", key, name));
}

bool Node::has_necessary_inputs() const {
	for (auto p: in_ports)
		if (!(p->flags & PortFlags::Optional) and !p->has_value())
			return false;
	return true;
}


xhui::Panel* Node::create_panel() {
	return new DefaultNodePanel(this);
}



} // graph