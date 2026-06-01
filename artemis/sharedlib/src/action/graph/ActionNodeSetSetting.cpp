//
// Created by michi on 11/11/25.
//

#include "ActionNodeSetSetting.h"
#include <lib/dataflow/Graph.h>
#include <graph/Graph.h>

namespace artemis {

ActionNodeSetSetting::ActionNodeSetSetting(dataflow::Node* node, const string& key, const Any& value) {
	this->node = node;
	this->key = key;
	this->value = value;
}

void* ActionNodeSetSetting::execute(history::Data* d) {
	const auto temp = node->get(key);
	node->set(key, value);
	value = temp;
	return node;
}

void ActionNodeSetSetting::undo(history::Data* d) {
	execute(d);
}

bool ActionNodeSetSetting::absorb(Action* previous) {
	if (auto a = dynamic_cast<ActionNodeSetSetting*>(previous); a) {
		if (a->node != node or a->key != key)
			return false;
		return true;
	}
	return false;
}


}

