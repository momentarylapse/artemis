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

class RegularGrid : public ResourceNode {
public:
	RegularGrid() : ResourceNode("RegularGrid") {}

	void process() override;

	Setting<int> nx{this, "nx", 8};
	Setting<int> ny{this, "ny", 8};
	Setting<int> nz{this, "nz", 1};

	OutPort<artemis::data::RegularGrid> out{this, "out"};
	OutPort<Array<vec3>> out_points{this, "points"};
};

} // graph

#endif //REGULARGRIDSOURCE_H
