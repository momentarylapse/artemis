//
// Created by Michael Ankele on 2025-03-18.
//

#ifndef NUMBERLISTACCUMULATOR_H
#define NUMBERLISTACCUMULATOR_H

#include <lib/dataflow/Node.h>
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>

namespace artemis::graph {

class NumberListAccumulator : public dataflow::Node {
public:
	NumberListAccumulator() : Node("NumberListAccumulator") {}

	void process() override;

	dataflow::Setting<int> capacity{this, "capacity", 1000};

	dataflow::InPort<float> in_number{this, "number"};
	dataflow::OutPort<Array<float>> out_list{this, "list"};
};

} // graph

#endif //NUMBERLISTACCUMULATOR_H
