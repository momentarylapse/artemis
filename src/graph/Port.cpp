//
// Created by michi on 09.03.25.
//

#include "Port.h"
#include "Node.h"

namespace graph {

bool operator&(PortFlags a, PortFlags b) {
	return ((int)a) & ((int)b);
}

OutPortBase::OutPortBase(Node* owner, const string& name, const kaba::Class* class_, PortFlags flags) {
	this->owner = owner;
	this->name = name;
	this->class_ = class_;
	this->flags = flags;
	port_index = owner->out_ports.num;
	owner->out_ports.add(this);
}

void OutPortBase::mutated() {
	for (auto t: targets)
		t->owner->dirty = true;
}


InPortBase::InPortBase(Node* owner, const string& name, const kaba::Class* class_, PortFlags flags) {
	this->owner = owner;
	this->name = name;
	this->class_ = class_;
	this->flags = flags;
	owner->in_ports.add(this);
}

void InPortBase::mutated() {
	if (source)
		source->mutated();
}

bool InPortBase::has_value() const {
	if (source)
		return source->has_value();
	return false;
}


} // graph