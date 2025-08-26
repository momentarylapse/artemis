//
// Created by michi on 09.03.25.
//

#pragma once

#include <lib/base/base.h>
#include <lib/base/error.h>
#include "Node.h"
#include "Port.h"

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
	Array<InPortForward*> _in_ports_forward;
	Array<OutPortForward*> _out_ports_forward;

	void clear();

	void add_node(Node* node);
	void remove_node(Node* node);

	void add_in_port_forward(InPortBase* target);
	void add_out_port_forward(OutPortBase* target);

	base::expected<int> connect(OutPortBase& source, InPortBase& sink);
	base::expected<int> connect(const CableInfo& c);
	void unconnect(OutPortBase& source, InPortBase& sink);
	void unconnect(const CableInfo& c);

	Array<CableInfo> cables() const;

	bool iterate();
	void on_process() override;

	// trigger all updates (for simulations)
	void reset_state();
};

bool port_type_match(const OutPortBase& source, const InPortBase& sink);

} // dataflow

