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
	std::function<bool()> has_value;
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
};


template<class T>
class OutPort;

template<class T>
class InPort : public InPortBase {
public:
	InPort(Node* owner, const string& name, PortFlags flags = PortFlags::None) : InPortBase(owner, name, get_class<T>(), flags) {}
	const T* value() const {
		if (sources.num == 0)
			return nullptr;
		auto typed_source = reinterpret_cast<OutPort<T>*>(sources[0]);
		if (!typed_source->value)
			return nullptr;
		return &(*typed_source->value);
	}
	const Array<T*> values() const {
		Array<T*> r;
		for (auto s: sources) {
			auto typed_source = reinterpret_cast<OutPort<T>*>(s);
			if (typed_source->value)
				r.add(&(*typed_source->value));
		}
		return r;
	}
};

template<class T>
class OutPort : public OutPortBase {
public:
	OutPort(Node* owner, const string& name, PortFlags flags = PortFlags::None) : OutPortBase(owner, name, get_class<T>(), flags) {
		has_value = [this] {
			return value.has_value();
		};
	}
	void operator()(const T& v) {
		value = v;
		mutated();
	}
	base::optional<T> value;
};

} // graph

