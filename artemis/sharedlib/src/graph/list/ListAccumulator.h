//
// Created by Michael Ankele on 2025-03-18.
//

#pragma once

#include <lib/dataflow/Node.h>
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>

namespace artemis::graph {

class ListAccumulator : public dataflow::Node {
public:
	ListAccumulator() : Node("ListAccumulator") {}

	void on_process() override;

	dataflow::Setting<int> capacity{this, "capacity", 1000};

	dataflow::InPort<double> in_number{this, "number"};
	dataflow::OutPort<Array<double>> out_list{this, "list"};
};

} // graph
