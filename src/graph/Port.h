//
// Created by michi on 09.03.25.
//

#ifndef PORT_H
#define PORT_H

#include <lib/base/base.h>
#include <lib/base/optional.h>

namespace graph {

class Node;

enum class PortDirection {
	Input,
	Output
};

class PortBase {
public:
	explicit PortBase(Node* owner, const string& name, PortDirection direction);
	Node* owner;
	string name;
	PortDirection direction;
};

template<class T>
class InPort : public PortBase {
public:
	InPort(Node* owner, const string& name) : PortBase(owner, name, PortDirection::Input) {}
	//T value() const;
};

template<class T>
class OutPort : public PortBase {
public:
	OutPort(Node* owner, const string& name) : PortBase(owner, name, PortDirection::Output) {}
	void operator()(const T& v) {
		value = v;
	}
	base::optional<T> value;
};

} // graph

#endif //PORT_H
