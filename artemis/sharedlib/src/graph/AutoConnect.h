//
// Created by Michael Ankele on 2025-03-17.
//

#pragma once

#include <lib/base/error.h>

namespace dataflow {
	struct CableInfo;
	class Node;
	class Graph;
}

namespace artemis::graph {

struct GraphUpdate {
	Array<dataflow::Node*> nodes;
	Array<dataflow::CableInfo> cables;
};

base::expected<GraphUpdate> find_auto_connect(dataflow::Graph* g, const dataflow::CableInfo& c);
base::expected<int> auto_connect(dataflow::Graph* g, const dataflow::CableInfo& c);

} // graph
