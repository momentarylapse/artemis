//
// Created by michi on 2025-11-10.
//

#pragma once

#include <action/Action.h>

namespace dataflow {
	class Node;
	class Graph;
	class InPortBase;
	class OutPortBase;
}

namespace artemis {

class ActionGraphConnect : public Action {
public:
	ActionGraphConnect(dataflow::OutPortBase* source, dataflow::InPortBase* sink);
	string name() const override { return "connect"; }

	void* execute(Data* d) override;
	void undo(Data* d) override;

	dataflow::OutPortBase* source;
	dataflow::InPortBase* sink;
};

}
