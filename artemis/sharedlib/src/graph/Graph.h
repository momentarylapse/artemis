//
// Created by Michael Ankele on 2025-03-31.
//

#pragma once

#include <lib/dataflow/Graph.h>
#include <lib/base/pointer.h>
#include <lib/history/Data.h>

namespace artemis::graph {

void iterate_simulation(Session* s);

class DataGraph : public history::Data {
public:
	explicit DataGraph(Session* session);
	void reset() override;

	Session* session;
	owned<dataflow::Graph> graph;

	// actions
	void add_node(dataflow::Node* node);
	void remove_nodes(const Array<dataflow::Node*>& nodes);
	void node_set_setting(dataflow::Node* node, const string& key, const Any& value);
	base::expected<int> connect(const dataflow::CableInfo& cable);
	void unconnect(const dataflow::CableInfo& cable);
	base::expected<int> auto_connect(const dataflow::CableInfo& c);
	dataflow::Graph* group_nodes(const Array<dataflow::Node*>& selected_nodes);
	void move_nodes(const Array<dataflow::Node*>& selected_nodes, const vec2& delta);
};

}
