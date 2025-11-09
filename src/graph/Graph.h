//
// Created by Michael Ankele on 2025-03-31.
//

#pragma once

#include <lib/dataflow/Graph.h>
#include <lib/base/pointer.h>
#include <data/Data.h>

namespace artemis::graph {

class Graph : public dataflow::Graph {
public:
	explicit Graph(Session* session);

	void iterate_simulation();

	Graph* group_nodes(const base::set<Node*>& selected_nodes);
};


class DataGraph : public Data {
public:
	explicit DataGraph(Session* session);
	void reset() override;

	owned<Graph> graph;
};

}
