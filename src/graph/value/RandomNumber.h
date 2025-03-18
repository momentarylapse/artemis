//
// Created by Michael Ankele on 2025-03-18.
//

#ifndef RANDOMNUMBER_H
#define RANDOMNUMBER_H

#include <graph/Node.h>
#include <graph/Port.h>

namespace graph {

class RandomNumber : public Node {
public:
	RandomNumber() : Node("RandomNumber") {
		flags = NodeFlags::TimeDependent;
	}

	void process() override;

	OutPort<float> out_number{this, "number"};
};

} // graph

#endif //RANDOMNUMBER_H
