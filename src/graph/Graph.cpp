//
// Created by Michael Ankele on 2025-03-31.
//

#include "Graph.h"
#include <Session.h>
#include <lib/dataflow/Graph.h>
#include <lib/dataflow/Node.h>
#include <action/graph/ActionGraphAddNode.h>
#include <action/graph/ActionGraphRemoveNode.h>
#include <action/graph/ActionGraphConnect.h>
#include <action/graph/ActionGraphUnconnect.h>
#include <action/graph/ActionNodeSetSetting.h>

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

void DataGraph::add_node(dataflow::Node* node) {
	execute(new ActionGraphAddNode(node));
}

void DataGraph::remove_node(dataflow::Node* node) {
	execute(new ActionGraphRemoveNode(node));
}

void DataGraph::node_set_setting(dataflow::Node* node, const string& key, const Any& value) {
	execute(new ActionNodeSetSetting(node, key, value));
}

void DataGraph::unconnect(const dataflow::CableInfo& cable) {
	execute(new ActionGraphUnconnect(cable.source, cable.sink));
}


}
