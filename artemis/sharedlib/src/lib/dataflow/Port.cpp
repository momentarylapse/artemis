//
// Created by michi on 09.03.25.
//

#include "Port.h"
#include "Node.h"

namespace dataflow {

PortFlags operator|(PortFlags a, PortFlags b) {
	return (PortFlags)((int)a | (int)b);
}

bool operator&(PortFlags a, PortFlags b) {
	return (int)a & (int)b;
}

OutPortBase::OutPortBase(Node* owner, const string& name, const kaba::Class* type, void* p, PortFlags flags) {
	this->owner = owner;
	this->name = name;
	this->type = type;
	generic_value_pointer = p;
	this->flags = flags;
//	port_index = owner->out_ports.num;
	if (owner)
		owner->out_ports.add(this);
}

string OutPortBase::full_name() const {
	return owner->name + "/" + name;
}


void OutPortBase::generic_set(void *p) {
	generic_value_pointer = p;
	has_value = true;
	mutated();
}

void OutPortBase::mutated() {
	for (auto t: targets)
		t->on_value_changed();
}


InPortBase::InPortBase(Node* owner, const string& name, const kaba::Class* type_filter, PortFlags flags) {
	this->owner = owner;
	this->name = name;
	this->type = type_filter;
	this->flags = flags;
	if (owner)
		owner->in_ports.add(this);
}

string InPortBase::full_name() const {
	return owner->name + "/" + name;
}

void InPortBase::on_value_changed() {
	if (link_partner)
		link_partner->mutated();
	owner->on_input_changed(this);
}


void InPortBase::mutated() {
	for (auto s: sources)
		s->mutated();
}

bool InPortBase::has_value() const {
	/*for (auto s: sources)
		if (s->has_value)
			return true;
	return false;*/
	return generic_values().num >= 1;
}

Array<GenericData> InPortBase::generic_values() const {
	Array<GenericData> r;
	for (auto s: sources) {
		if (s->link_partner) {
			r.append(s->link_partner->generic_values());
		} else {
			r.add({s->type, s->generic_value_pointer});
		}
	}
	return r;
}

}
