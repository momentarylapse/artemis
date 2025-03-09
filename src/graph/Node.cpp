//
// Created by michi on 09.03.25.
//

#include "Node.h"

namespace graph {

Node::Node(const string& name) {
	this->name = name;
	pos = vec2(0, 0);
}

} // graph