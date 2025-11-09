//
// Created by Michael Ankele on 2025-03-17.
//

#pragma once

#include <lib/base/error.h>

namespace dataflow {
	struct CableInfo;
	class Graph;
}

namespace artemis::graph {

base::expected<int> auto_connect(dataflow::Graph* g, const dataflow::CableInfo& c);

} // graph
