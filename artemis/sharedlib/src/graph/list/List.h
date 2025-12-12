//
// Created by Michael Ankele on 2025-07-26.
//

#pragma once

#include <lib/dataflow/ResourceNode.h>
#include <lib/dataflow/Setting.h>

namespace artemis::graph {

class List : public dataflow::ResourceNode<Array<double>> {
public:
	List() : ResourceNode("List") {}

	void on_process() override;

	dataflow::Setting<Array<double>> list{this, "list", {0.0, 1.0, 2.0}};
};

} // graph

