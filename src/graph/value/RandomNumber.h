//
// Created by Michael Ankele on 2025-03-18.
//

#ifndef RANDOMNUMBER_H
#define RANDOMNUMBER_H

#include <lib/dataflow/Node.h>
#include <lib/dataflow/Port.h>

namespace artemis::graph {

class RandomNumber : public dataflow::Node {
public:
	RandomNumber() : Node("RandomNumber") {
		flags = dataflow::NodeFlags::TimeDependent;
	}

	void on_process() override;

	dataflow::OutPort<float> out_number{this, "number"};
};

} // graph

#endif //RANDOMNUMBER_H
