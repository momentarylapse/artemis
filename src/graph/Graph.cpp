//
// Created by michi on 09.03.25.
//

#include "Graph.h"

namespace graph {

Graph::Graph(Session* s) {
	session = s;
}


void Graph::add_node(Node* node) {
	nodes.add(node);
}


} // graph