//
// Created by michi on 09.03.25.
//

#pragma once

#include <lib/base/base.h>
#include <lib/base/error.h>
#include <lib/pattern/Observable.h>
#include "Node.h"

class Session;

namespace dataflow {

class InPortBase;
class OutPortBase;

struct CableInfo {
	Node* source;
	int source_port;
	Node* sink;
	int sink_port;
};

class Graph : public Node {
public:
	Graph();

	Array<Node*> nodes;

	void clear();

	void add_node(Node* node);
	void remove_node(Node* node);

	base::expected<int> connect(OutPortBase& source, InPortBase& sink);
	base::expected<int> connect(const CableInfo& c);
	void unconnect(OutPortBase& source, InPortBase& sink);
	void unconnect(const CableInfo& c);

	Array<CableInfo> cables() const;

	void reset_state();
	bool iterate();
};

bool port_type_match(const OutPortBase& source, const InPortBase& sink);

} // dataflow

