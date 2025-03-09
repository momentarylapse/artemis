//
// Created by michi on 09.03.25.
//

#ifndef PORT_H
#define PORT_H

#include <lib/base/base.h>
#include <lib/base/optional.h>
#include <lib/kaba/kaba.h>
#include <data/mesh/PolygonMesh.h>

namespace graph {

class Node;

// TODO type registry (map typeinfo -> kaba.Class)
template<class T>
	const kaba::Class* to_class() {
	if constexpr (std::is_same_v<T, float>)
		return kaba::TypeFloat32;
	if constexpr (std::is_same_v<T, int>)
		return kaba::TypeInt32;
	if constexpr (std::is_same_v<T, string>)
		return kaba::TypeString;
	if constexpr (std::is_same_v<T, PolygonMesh>)
		return (const kaba::Class*)0x05;
	return nullptr;
}

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
	InPort(Node* owner, const string& name) : InPortBase(owner, name, to_class<T>()) {}
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
	OutPort(Node* owner, const string& name) : OutPortBase(owner, name, to_class<T>()) {}
	void operator()(const T& v) {
		value = v;
	}
	base::optional<T> value;
};

} // graph

#endif //PORT_H
