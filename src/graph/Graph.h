//
// Created by michi on 09.03.25.
//

#ifndef GRAPH_H
#define GRAPH_H
#include <gui/Node.h>
#include <lib/base/array.h>

class Session;

namespace graph {

class Node;
class InPortBase;
class OutPortBase;

class Graph {
public:
	explicit Graph(Session* session);

	Session* session;

	Array<Node*> nodes;

	void add_node(Node* node);

	void connect(OutPortBase& out, InPortBase& in);
	void connect(Node* source, int source_port, Node* sink, int sink_port);

	void iterate();
};

} // graph

#endif //GRAPH_H
