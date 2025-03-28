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
	Setting<float> dx{this, "dx", 1.0f};
	Setting<float> dy{this, "dy", 1.0f};
	Setting<float> dz{this, "dz", 1.0f};

	OutPort<artemis::data::RegularGrid> out{this, "out"};
	OutPort<Array<vec3>> out_vertices{this, "vertices"};
	OutPort<Array<vec3>> out_cell_centers{this, "cell_centers"};
};

} // graph

#endif //REGULARGRIDSOURCE_H
