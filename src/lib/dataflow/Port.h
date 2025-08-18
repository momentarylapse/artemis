//
// Created by michi on 09.03.25.
//

#pragma once

#include "Node.h"
#include "Type.h"
#include <lib/base/base.h>
#include <lib/base/optional.h>
#include <lib/kaba/kaba.h>

namespace dataflow {

class Node;
class InPortBase;

enum class PortFlags {
	None = 0,
	Mutable = 1,
	Optional = 2,
	Multi = 4
};
bool operator&(PortFlags a, PortFlags b);

class OutPortBase {
public:
	explicit OutPortBase(Node* owner, const string& name, const kaba::Class* class_, PortFlags flags);
	Node* owner;
	int port_index;
	string name;
	const kaba::Class* class_;
	PortFlags flags;
	Array<InPortBase*> targets;
	void mutated();
	bool has_value = false;
	void* generic_value_pointer = nullptr;
};

class InPortBase {
public:
	explicit InPortBase(Node* owner, const string& name, const kaba::Class* class_, PortFlags flags);
	Node* owner;
	string name;
	const kaba::Class* class_;
	PortFlags flags;
	Array<OutPortBase*> sources;
	void mutated();
	bool has_value() const;
	Array<const kaba::Class*> types() const;
};


template<class T>
class OutPort;

template<class T>
class InPort : public InPortBase {
public:
	InPort(Node* owner, const string& name, PortFlags flags = PortFlags::None) : InPortBase(owner, name, get_class<T>(), flags) {}
	const T* value() const {
		for (auto s: sources)
			if (s->has_value)
				return reinterpret_cast<T*>(s->generic_value_pointer);
		return nullptr;
	}
	Array<T*> values() const {
		Array<T*> r;
		for (auto s: sources)
			if (s->has_value)
				r.add(reinterpret_cast<T*>(s->generic_value_pointer));
		return r;
	}
};

template<class T>
class OutPort : public OutPortBase {
public:
	OutPort(Node* owner, const string& name, PortFlags flags = PortFlags::None) : OutPortBase(owner, name, get_class<T>(), flags) {
		generic_value_pointer = &_value;
	}
	void operator()(const T& v) {
		_value = v;
		has_value = true;
		mutated();
	}
	T& value() {
		return _value;
	}
private:
	T _value;
};

} // graph

