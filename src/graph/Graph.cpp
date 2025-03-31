//
// Created by Michael Ankele on 2025-03-31.
//

#include "Graph.h"
#include <lib/dataflow/Node.h>

namespace artemis {
namespace graph {

float _current_simulation_time_ = 0;
float _current_simulation_dt_ = 0;

Graph::Graph(Session* s) {
	session = s;
}

void Graph::iterate_simulation(float dt) {
	_current_simulation_dt_ = dt;
	_current_simulation_time_ += dt;
	for (auto n: nodes)
		if (n->flags & dataflow::NodeFlags::TimeDependent and n->has_necessary_inputs()) {
			n->process();
			n->dirty = false;
		}
}

} // graph
} // artemis