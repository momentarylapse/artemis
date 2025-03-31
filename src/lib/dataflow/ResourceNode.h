//
// Created by Michael Ankele on 2025-03-31.
//

#pragma once

#include "Node.h"
#include "Port.h"

namespace dataflow {


template<class T>
class ResourceNode : public Node {
public:
	explicit ResourceNode(const string& name) : Node(name) {
		flags = NodeFlags::Resource;
	}

	OutPort<T> out{this, "out", PortFlags::Mutable};
};

} // dataflow

