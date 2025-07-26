//
// Created by Michael Ankele on 2025-07-26.
//

#pragma once

#include <lib/dataflow/ResourceNode.h>
#include <lib/dataflow/Setting.h>

namespace artemis::graph {

	class ListToVectors : public dataflow::Node {
	public:
		ListToVectors() : Node("ListToVectors") {}

		void on_process() override;

		dataflow::Setting<int> first{this, "first", 0};
		dataflow::Setting<int> stride{this, "stride", 3};

		dataflow::InPort<Array<double>> in_numbers{this, "numbers"};
		dataflow::OutPort<Array<vec3>> out_vectors{this, "vectors"};
	};

} // graph

