//
// Created by michi on 09.03.25.
//

#ifndef NODEFACTORY_H
#define NODEFACTORY_H

#include <lib/base/base.h>

class Session;

namespace dataflow {
	class Node;
	enum class NodeCategory;
}

namespace artemis::graph {

void init_factory();
Array<string> enumerate_nodes(dataflow::NodeCategory category);
dataflow::Node* create_node(Session* s, const string& name);

} // graph

#endif //NODEFACTORY_H
