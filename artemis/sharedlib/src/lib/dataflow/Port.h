//
// Created by michi on 09.03.25.
//

#pragma once

#include "Node.h"
#include "Type.h"
#include <lib/base/base.h>
#include <lib/base/xparam.h>

namespace dataflow {

class Node;
class InPortBase;

enum class PortFlags {
	None = 0,
	Mutable = 1,
	Optional = 2,
	Multi = 4,
	Forwarding = 8
};
PortFlags operator|(PortFlags a, PortFlags b);
bool operator&(PortFlags a, PortFlags b);

class OutPortBase {
public:
	explicit OutPortBase(Node* owner, const string& name, const kaba::Class* type, void* p, PortFlags flags);
	Node* owner;
//	int port_index;
	string name;
	const kaba::Class* type; // never null!
	PortFlags flags;
	Array<InPortBase*> targets;
	InPortBase* link_partner = nullptr;
	void generic_set(void* p);
	void mutated(); // send new value, notify connected sinks
	bool has_value = false;
	void* generic_value_pointer = nullptr;
	string full_name() const;
};

struct GenericData {
	const kaba::Class* type;
	void* p;
};

class InPortBase {
public:
	explicit InPortBase(Node* owner, const string& name, const kaba::Class* type_filter, PortFlags flags);
	Node* owner;
	string name;
	const kaba::Class* type; // filter, can be null
	PortFlags flags;
	Array<OutPortBase*> sources;
	OutPortBase* link_partner = nullptr;
	void mutated(); // for nodes that can overwrite input
	bool has_value() const;
	Array<GenericData> generic_values() const;
	string full_name() const;

	void on_value_changed(); // from connected out port
};


template<class T>
class OutPort;

template<class T>
class InPort : public InPortBase {
public:
	InPort(Node* owner, const string& name, PortFlags flags = PortFlags::None) : InPortBase(owner, name, get_class<T>(), flags) {}
	const T* value() const {
		for (auto& v: generic_values())
			return reinterpret_cast<T*>(v.p);
		return nullptr;
	}
	Array<T*> values() const {
		Array<T*> r;
		for (auto& v: generic_values())
			r.add(reinterpret_cast<T*>(v.p));
		return r;
	}
};

template<class T>
class OutPort : public OutPortBase {
public:
	OutPort(Node* owner, const string& name, PortFlags flags = PortFlags::None) : OutPortBase(owner, name, get_class<T>(), &_value, flags) {
	}
	void operator()(const typename base::xparam<T>::t v) {
		_value = v;
		generic_set(&_value);
	}
	T& value() {
		return _value;
	}
private:
	T _value;
};

}

