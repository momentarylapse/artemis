//
// Created by michi on 2025-11-10.
//

#pragma once

#include <lib/history/Action.h>

namespace dataflow {
	class Node;
}

namespace artemis {

class ActionGraphAddNode : public history::Action {
public:
	explicit ActionGraphAddNode(dataflow::Node* node);
	string name() const override { return "graph-add-node"; }
	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

	dataflow::Node* node;
};

}
