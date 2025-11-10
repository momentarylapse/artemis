//
// Created by Michael Ankele on 2025-03-31.
//

#pragma once

#include <lib/dataflow/Graph.h>
#include <lib/base/pointer.h>
#include <data/Data.h>

namespace artemis::graph {

void iterate_simulation(Session* s);

class DataGraph : public Data {
public:
	explicit DataGraph(Session* session);
	void reset() override;

	owned<dataflow::Graph> graph;

	// actions
	void add_node(dataflow::Node* node);
	void remove_node(dataflow::Node* node);
	void node_set_setting(dataflow::Node* node, const string& key, const Any& value);
	void connect(const dataflow::CableInfo& cable);
	void unconnect(const dataflow::CableInfo& cable);
};

}
