//
// Created by michi on 09.03.25.
//

#ifndef GRAPH_H
#define GRAPH_H

#include <lib/base/base.h>
#include <lib/pattern/Observable.h>

class Session;

namespace graph {

class Node;
class InPortBase;
class OutPortBase;

struct CableInfo {
	Node* source;
	int source_port;
	Node* sink;
	int sink_port;
};

class Graph : public obs::Node<VirtualBase> {
public:
	explicit Graph(Session* session);

	Session* session;

	Array<graph::Node*> nodes;

	void clear();

	void add_node(graph::Node* node);
	void remove_node(graph::Node* node);

	void connect(OutPortBase& source, InPortBase& sink);
	void connect(const CableInfo& c);
	void unconnect(OutPortBase& source, InPortBase& sink);
	void unconnect(const CableInfo& c);

	Array<CableInfo> cables() const;

	void reset_state();
	bool iterate();
	void iterate_simulation(float dt);
};

} // graph

#endif //GRAPH_H
