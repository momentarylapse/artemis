//
// Created by michi on 11/15/25.
//

#pragma once

#include <lib/history/MergableAction.h>
#include <lib/math/vec2.h>

namespace dataflow {
	class Node;
}

namespace artemis {

class ActionNodesMove : public history::MergableAction {
public:
	explicit ActionNodesMove(const Array<dataflow::Node*>& nodes, const vec2& offset);
	string name() const override { return "graph-nodes-move"; }
	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

	bool absorb(Action* previous) override;

	Array<dataflow::Node*> nodes;
	Array<vec2> pos;
	vec2 offset;
};

}
