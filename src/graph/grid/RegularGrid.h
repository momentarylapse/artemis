//
// Created by Michael Ankele on 2025-03-10.
//

#ifndef REGULARGRIDSOURCE_H
#define REGULARGRIDSOURCE_H

#include <lib/dataflow/ResourceNode.h>
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>
#include <data/grid/RegularGrid.h>

namespace artemis::graph {

class RegularGrid : public dataflow::ResourceNode<data::RegularGrid> {
public:
	RegularGrid() : ResourceNode("RegularGrid") {}

	void on_process() override;

	dataflow::Setting<int> nx{this, "nx", 8};
	dataflow::Setting<int> ny{this, "ny", 8};
	dataflow::Setting<int> nz{this, "nz", 1};
	dataflow::Setting<double> lx{this, "lx", 8.0};
	dataflow::Setting<double> ly{this, "ly", 8.0};
	dataflow::Setting<double> lz{this, "lz", 1.0};

	dataflow::OutPort<Array<vec3>> out_vertices{this, "vertices"};
	dataflow::OutPort<Array<vec3>> out_cell_centers{this, "cell_centers"};
};

} // graph

#endif //REGULARGRIDSOURCE_H
