//
// Created by Michael Ankele on 2025-03-17.
//

#pragma once

#include <lib/base/error.h>

namespace dataflow {
	struct CableInfo;
}

namespace artemis::graph {

class Graph;

base::expected<int> auto_connect(Graph* g, const dataflow::CableInfo& c);

} // graph
