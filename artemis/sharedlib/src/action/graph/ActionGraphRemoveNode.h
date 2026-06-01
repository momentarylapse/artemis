//
// Created by michi on 11/11/25.
//

#pragma once

#include <lib/history/Action.h>

namespace dataflow {
	class Node;
}

namespace artemis {

class ActionGraphRemoveNode : public history::Action {
public:
	explicit ActionGraphRemoveNode(dataflow::Node* node);
	string name() const override { return "graph-remove-node"; }
	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

	dataflow::Node* node;
};

}
