//
// Created by michi on 09.03.25.
//

#ifndef PORT_H
#define PORT_H

#include "Node.h"
#include <lib/base/base.h>
#include <lib/base/optional.h>
#include <lib/kaba/kaba.h>
#include <plugins/PluginManager.h>

namespace graph {

class Node;
class InPortBase;

enum class PortFlags {
	None = 0,
	Mutable = 1,
	Optional = 2
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
};

class InPortBase {
public:
	explicit InPortBase(Node* owner, const string& name, const kaba::Class* class_, PortFlags flags);
	Node* owner;
	string name;
	const kaba::Class* class_;
	PortFlags flags;
	OutPortBase* source = nullptr;
	void mutated();
};


template<class T>
class OutPort;

template<class T>
class InPort : public InPortBase {
public:
	InPort(Node* owner, const string& name, PortFlags flags = PortFlags::None) : InPortBase(owner, name, artemis::get_class<T>(), flags) {}
	const T* value() const {
		if (!source)
			return nullptr;
		auto typed_source = reinterpret_cast<OutPort<T>*>(source);
		if (!typed_source->value)
			return nullptr;
		return &(*typed_source->value);
	}
};

template<class T>
class OutPort : public OutPortBase {
public:
	OutPort(Node* owner, const string& name, PortFlags flags = PortFlags::None) : OutPortBase(owner, name, artemis::get_class<T>(), flags) {}
	void operator()(const T& v) {
		value = v;
		mutated();
	}
	base::optional<T> value;
};

} // graph

#endif //PORT_H
