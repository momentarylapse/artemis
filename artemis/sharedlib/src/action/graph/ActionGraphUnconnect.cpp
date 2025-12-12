//
// Created by michi on 2025-11-10.
//

#include "ActionGraphUnconnect.h"
#include <lib/dataflow/Graph.h>
#include <graph/Graph.h>

namespace artemis {

ActionGraphUnconnect::ActionGraphUnconnect(dataflow::OutPortBase* source, dataflow::InPortBase* sink) {
	this->source = source;
	this->sink = sink;
}

void* ActionGraphUnconnect::execute(Data* d) {
	auto g = dynamic_cast<graph::DataGraph*>(d);
	g->graph->unconnect({source, sink});
	return nullptr;
}

void ActionGraphUnconnect::undo(Data* d) {
	auto g = dynamic_cast<graph::DataGraph*>(d);
	g->graph->connect({source, sink});
}

}
