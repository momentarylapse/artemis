//
// Created by michi on 09.03.25.
//

#include "Port.h"
#include "Node.h"

namespace dataflow {

bool operator&(PortFlags a, PortFlags b) {
	return ((int)a) & ((int)b);
}

OutPortBase::OutPortBase(Node* owner, const string& name, const kaba::Class* type, void* p, PortFlags flags) {
	this->owner = owner;
	this->name = name;
	this->type = type;
	generic_value_pointer = p;
	this->flags = flags;
	port_index = owner->out_ports.num;
	owner->out_ports.add(this);
}

void OutPortBase::generic_set(void *p) {
	generic_value_pointer = p;
	has_value = true;
	mutated();
}

void OutPortBase::mutated() {
	for (auto t: targets)
		t->owner->on_input_changed(t);
}


InPortBase::InPortBase(Node* owner, const string& name, const kaba::Class* type_filter, PortFlags flags) {
	this->owner = owner;
	this->name = name;
	this->type = type_filter;
	this->flags = flags;
	owner->in_ports.add(this);
}

void InPortBase::mutated() {
	for (auto s: sources)
		s->mutated();
}

bool InPortBase::has_value() const {
	for (auto s: sources)
		if (s->has_value)
			return true;
	return false;
}

Array<GenericData> InPortBase::generic_values() const {
	Array<GenericData> r;
	for (auto s: sources)
		if (s->has_value)
			r.add({s->type, s->generic_value_pointer});
	return r;
}

OutPortForward::OutPortForward(Node* owner, OutPortBase *target) : OutPortBase(owner, target->name, target->type, target->generic_value_pointer, target->flags) {
	this->target = target;
}

InPortForward::InPortForward(Node* owner, InPortBase *target) : InPortBase(owner, target->name, target->type, target->flags) {
	this->target = target;
}



} // graph