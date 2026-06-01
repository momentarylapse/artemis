//
// Created by michi on 11/11/25.
//

#pragma once

#include <lib/history/MergableAction.h>
#include <lib/any/any.h>

namespace dataflow {
	class Node;
}

namespace artemis {

class ActionNodeSetSetting : public history::MergableAction {
public:
	explicit ActionNodeSetSetting(dataflow::Node* node, const string& key, const Any& value);
	string name() const override { return "graph-node-set-setting"; }
	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

	bool absorb(Action* previous) override;

	dataflow::Node* node;
	string key;
	Any value;
};

}