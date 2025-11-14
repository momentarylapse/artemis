//
// Created by Michael Ankele on 2025-03-31.
//

#include "Graph.h"
#include "AutoConnect.h"
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

void DataGraph::remove_nodes(const Array<dataflow::Node*>& nodes) {
	action_manager->begin_group("delete");
	for (const auto& c: graph->cables())
		if (nodes.find(c.source->owner) >= 0 or nodes.find(c.sink->owner) >= 0)
			unconnect(c);
	for (auto n: nodes)
		execute(new ActionGraphRemoveNode(n));
	action_manager->end_group();
}

void DataGraph::node_set_setting(dataflow::Node* node, const string& key, const Any& value) {
	execute(new ActionNodeSetSetting(node, key, value));
}

base::expected<int> DataGraph::connect(const dataflow::CableInfo& cable) {
	try {
		execute(new ActionGraphConnect(cable.source, cable.sink));
	} catch (const Exception& e) {
		return base::Error{e.message()};
	}
	return 0;
}

void DataGraph::unconnect(const dataflow::CableInfo& cable) {
	execute(new ActionGraphUnconnect(cable.source, cable.sink));
}

base::expected<int> DataGraph::auto_connect(const dataflow::CableInfo& c) {
	// solve
	auto update = find_auto_connect(graph.get(), c);
	if (!update)
		return update.error();

	// apply
	action_manager->begin_group("update");
	for (auto n: update->nodes)
		add_node(n);
	for (const auto& cc: update->cables)
		if (auto r = connect(cc); !r) {
			action_manager->end_group(); // TODO abort!
			return r;
		}
	action_manager->end_group();
	return 0;
}


}
