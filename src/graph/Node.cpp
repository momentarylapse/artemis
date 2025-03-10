//
// Created by michi on 09.03.25.
//

#include "Node.h"
#include <lib/os/msg.h>
#include "Setting.h"

namespace graph {

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


} // graph