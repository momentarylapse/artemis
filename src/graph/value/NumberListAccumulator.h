//
// Created by Michael Ankele on 2025-03-18.
//

#ifndef NUMBERLISTACCUMULATOR_H
#define NUMBERLISTACCUMULATOR_H

#include <graph/Node.h>
#include <graph/Port.h>
#include <graph/Setting.h>

namespace graph {

class NumberListAccumulator : public Node {
public:
	NumberListAccumulator() : Node("NumberListAccumulator") {}

	void process() override;

	Setting<int> capacity{this, "capacity", 1000};

	InPort<float> in_number{this, "number"};
	OutPort<Array<float>> out_list{this, "list"};
};

} // graph

#endif //NUMBERLISTACCUMULATOR_H
