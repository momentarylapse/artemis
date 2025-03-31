//
// Created by Michael Ankele on 2025-03-10.
//

#ifndef REGULARGRIDSOURCE_H
#define REGULARGRIDSOURCE_H

#include <lib/dataflow/Node.h>
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>
#include <data/grid/RegularGrid.h>

namespace artemis::graph {

class RegularGrid : public dataflow::ResourceNode {
public:
	RegularGrid() : ResourceNode("RegularGrid") {}

	void process() override;

	dataflow::Setting<int> nx{this, "nx", 8};
	dataflow::Setting<int> ny{this, "ny", 8};
	dataflow::Setting<int> nz{this, "nz", 1};
	dataflow::Setting<float> dx{this, "dx", 1.0f};
	dataflow::Setting<float> dy{this, "dy", 1.0f};
	dataflow::Setting<float> dz{this, "dz", 1.0f};

	dataflow::OutPort<data::RegularGrid> out{this, "out"};
	dataflow::OutPort<Array<vec3>> out_vertices{this, "vertices"};
	dataflow::OutPort<Array<vec3>> out_cell_centers{this, "cell_centers"};
};

} // graph

#endif //REGULARGRIDSOURCE_H
