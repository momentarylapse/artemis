//
// Created by michi on 11/11/25.
//

#pragma once

#include <action/Action.h>
#include <lib/any/any.h>

namespace dataflow {
	class Node;
}

namespace artemis {

class ActionNodeSetSetting : public Action {
public:
	explicit ActionNodeSetSetting(dataflow::Node* node, const string& key, const Any& value);
	string name() const override { return "graph-node-set-setting"; }
	void* execute(Data* d) override;
	void undo(Data* d) override;

	dataflow::Node* node;
	string key;
	Any value;
};

}