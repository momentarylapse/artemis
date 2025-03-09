//
// Created by michi on 09.03.25.
//

#ifndef PORT_H
#define PORT_H

#include <lib/base/base.h>
#include <lib/base/optional.h>
#include <lib/kaba/kaba.h>
#include <plugins/PluginManager.h>

namespace graph {

class Node;

class OutPortBase {
public:
	explicit OutPortBase(Node* owner, const string& name, const kaba::Class* class_);
	Node* owner;
	string name;
	const kaba::Class* class_;
};

class InPortBase {
public:
	explicit InPortBase(Node* owner, const string& name, const kaba::Class* class_);
	Node* owner;
	string name;
	const kaba::Class* class_;
	OutPortBase* source = nullptr;
};


template<class T>
class OutPort;

template<class T>
class InPort : public InPortBase {
public:
	InPort(Node* owner, const string& name) : InPortBase(owner, name, artemis::get_class<T>()) {}
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
	OutPort(Node* owner, const string& name) : OutPortBase(owner, name, artemis::get_class<T>()) {}
	void operator()(const T& v) {
		value = v;
	}
	base::optional<T> value;
};

} // graph

#endif //PORT_H
