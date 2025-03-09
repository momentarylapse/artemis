//
// Created by michi on 09.03.25.
//

#include "Port.h"
#include "Node.h"

namespace graph {

PortBase::PortBase(Node* owner, const string& name, PortDirection direction) {
	this->owner = owner;
	this->name = name;
	this->direction = direction;
	if (direction == PortDirection::Input)
		owner->in_ports.add(this);
	else
		owner->out_ports.add(this);
}

} // graph