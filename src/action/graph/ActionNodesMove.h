//
// Created by michi on 11/15/25.
//

#pragma once

#include <action/MergableAction.h>
#include <lib/math/vec2.h>

namespace dataflow {
	class Node;
}

namespace artemis {

class ActionNodesMove : public MergableAction {
public:
	explicit ActionNodesMove(const Array<dataflow::Node*>& nodes, const vec2& offset);
	string name() const override { return "graph-nodes-move"; }
	void* execute(Data* d) override;
	void undo(Data* d) override;

	bool try_merge_into(Action* previous) override;

	Array<dataflow::Node*> nodes;
	Array<vec2> pos;
	vec2 offset;
};

}
