//
// Created by michi on 11/15/25.
//

#include "ActionNodesMove.h"
#include <lib/base/iter.h>
#include <lib/dataflow/Node.h>

namespace artemis {

ActionNodesMove::ActionNodesMove(const Array<dataflow::Node*>& nodes, const vec2& offset) {
	this->nodes = nodes;
	this->offset = offset;
}

void* ActionNodesMove::execute(Data* d) {
	pos.clear();
	for (auto n: nodes) {
		pos.add(n->pos);
		n->pos += offset;
	}
	return nullptr;
}

void ActionNodesMove::undo(Data* d) {
	for (auto&& [i, n]: enumerate(nodes)) {
		n->pos = pos[i];
	}
}

bool ActionNodesMove::try_merge_into(Action* previous) {
	if (auto a = dynamic_cast<ActionNodesMove*>(previous); a) {
		if (!a->is_recent_enough())
			return false;
		if (a->nodes != nodes)
			return false;

		a->offset += offset;
		a->time_stamp = time_stamp;
		return true;
	}
	return false;
}

}
