//
// Created by Michael Ankele on 2025-03-31.
//

#include "Graph.h"
#include <Session.h>
#include <lib/dataflow/Graph.h>
#include <lib/dataflow/Node.h>

namespace artemis {
	Session* current_session();
}

namespace artemis::graph {


void iterate_simulation(Session* s) {
	s->t += s->dt;
	for (auto n: s->graph->nodes)
		if (n->flags & dataflow::NodeFlags::TimeDependent and n->has_necessary_inputs()) {
			n->process();
			n->state = dataflow::NodeState::Complete;
		}
}


DataGraph::DataGraph(Session* session) : Data(session, -1) {
	graph = new dataflow::Graph();
}

void DataGraph::reset() {
	graph->clear();
}

}
