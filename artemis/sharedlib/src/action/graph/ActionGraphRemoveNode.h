//
// Created by michi on 11/11/25.
//

#pragma once

#include <action/Action.h>

namespace dataflow {
	class Node;
}

namespace artemis {

class ActionGraphRemoveNode : public Action {
public:
	explicit ActionGraphRemoveNode(dataflow::Node* node);
	string name() const override { return "graph-remove-node"; }
	void* execute(Data* d) override;
	void undo(Data* d) override;

	dataflow::Node* node;
};

}
