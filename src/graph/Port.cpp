//
// Created by michi on 09.03.25.
//

#include "Port.h"
#include "Node.h"

namespace graph {

OutPortBase::OutPortBase(Node* owner, const string& name, const kaba::Class* class_) {
	this->owner = owner;
	this->name = name;
	this->class_ = class_;
	port_index = owner->out_ports.num;
	owner->out_ports.add(this);
}

InPortBase::InPortBase(Node* owner, const string& name, const kaba::Class* class_) {
	this->owner = owner;
	this->name = name;
	this->class_ = class_;
	owner->in_ports.add(this);
}

} // graph