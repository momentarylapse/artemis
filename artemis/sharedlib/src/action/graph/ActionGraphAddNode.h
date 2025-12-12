//
// Created by michi on 2025-11-10.
//

#pragma once

#include <action/Action.h>

namespace dataflow {
	class Node;
}

namespace artemis {

class ActionGraphAddNode : public Action {
public:
	explicit ActionGraphAddNode(dataflow::Node* node);
	string name() const override { return "graph-add-node"; }
	void* execute(Data* d) override;
	void undo(Data* d) override;

	dataflow::Node* node;
};

}
