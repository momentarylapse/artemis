//
// Created by michi on 2025-11-10.
//

#include "ActionGraphConnect.h"
#include <lib/dataflow/Graph.h>
#include <graph/Graph.h>

namespace artemis {

ActionGraphConnect::ActionGraphConnect(dataflow::OutPortBase* source, dataflow::InPortBase* sink) {
	this->source = source;
	this->sink = sink;
}

void* ActionGraphConnect::execute(Data* d) {
	auto g = dynamic_cast<graph::DataGraph*>(d);
	g->graph->connect(*source, *sink);
	return nullptr;
}

void ActionGraphConnect::undo(Data* d) {
	auto g = dynamic_cast<graph::DataGraph*>(d);
	g->graph->unconnect(*source, *sink);
}

}
