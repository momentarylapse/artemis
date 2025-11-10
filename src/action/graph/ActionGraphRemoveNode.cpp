//
// Created by michi on 11/11/25.
//

#include "ActionGraphRemoveNode.h"
#include <lib/dataflow/Graph.h>
#include <graph/Graph.h>

namespace artemis {

ActionGraphRemoveNode::ActionGraphRemoveNode(dataflow::Node* node) {
	this->node = node;
}

void* ActionGraphRemoveNode::execute(Data* d) {
	auto g = dynamic_cast<graph::DataGraph*>(d);
	g->graph->remove_node(node);
	return nullptr;
}

void ActionGraphRemoveNode::undo(Data* d) {
	auto g = dynamic_cast<graph::DataGraph*>(d);
	g->graph->add_node(node);
}

}
