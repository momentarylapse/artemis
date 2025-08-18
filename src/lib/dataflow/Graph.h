//
// Created by michi on 09.03.25.
//

#pragma once

#include <lib/base/base.h>
#include <lib/pattern/Observable.h>

class Session;

namespace dataflow {

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
	Graph();

	int channel;
	Array<dataflow::Node*> nodes;

	void clear();

	void add_node(dataflow::Node* node);
	void remove_node(dataflow::Node* node);

	void connect(OutPortBase& source, InPortBase& sink);
	void connect(const CableInfo& c);
	void unconnect(OutPortBase& source, InPortBase& sink);
	void unconnect(const CableInfo& c);

	Array<CableInfo> cables() const;

	void reset_state();
	bool iterate();
};

bool port_type_match(const OutPortBase& source, const InPortBase& sink);

} // dataflow

