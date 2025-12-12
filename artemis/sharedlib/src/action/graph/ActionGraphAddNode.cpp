//
// Created by michi on 2025-11-10.
//

#include "ActionGraphAddNode.h"
#include <lib/dataflow/Graph.h>
#include <graph/Graph.h>

namespace artemis {

ActionGraphAddNode::ActionGraphAddNode(dataflow::Node* node) {
	this->node = node;
}

void* ActionGraphAddNode::execute(Data* d) {
	auto g = dynamic_cast<graph::DataGraph*>(d);
	g->graph->add_node(node);
	return node;
}

void ActionGraphAddNode::undo(Data* d) {
	auto g = dynamic_cast<graph::DataGraph*>(d);
	g->graph->remove_node(node);
}

}
