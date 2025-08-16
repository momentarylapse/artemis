//
// Created by Michael Ankele on 2025-03-31.
//

#include "Graph.h"
#include <lib/dataflow/Node.h>

namespace artemis {
namespace graph {

Graph::Graph(Session* s) {
	session = s;
	t = 0;
	dt = 0.1f;
}


void Graph::iterate_simulation() {
	t += dt;
	for (auto n: nodes)
		if (n->flags & dataflow::NodeFlags::TimeDependent and n->has_necessary_inputs()) {
			n->process();
			n->dirty = false;
		}
}

DataGraph::DataGraph(Session* session) : Data(session, -1), graph(session) {
}

void DataGraph::reset() {
	graph.clear();
}

} // graph
} // artemis