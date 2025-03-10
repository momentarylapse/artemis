//
// Created by Michael Ankele on 2025-03-10.
//

#ifndef REGULARGRIDSOURCE_H
#define REGULARGRIDSOURCE_H

#include "../Node.h"
#include <graph/Port.h>
#include <graph/Setting.h>
#include <data/grid/RegularGrid.h>

namespace graph {

class RegularGridSource : public Node {
public:
	RegularGridSource() : Node("RegularGridSource") {}

	void process() override;

	Setting<int> nx{this, "nx", 8};
	Setting<int> ny{this, "ny", 8};
	Setting<int> nz{this, "nz", 1};

	OutPort<RegularGrid> out{this, "out"};
};

} // graph

#endif //REGULARGRIDSOURCE_H
