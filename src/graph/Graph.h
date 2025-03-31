//
// Created by Michael Ankele on 2025-03-31.
//

#pragma once

#include <lib/dataflow/Graph.h>

namespace artemis::graph {

extern float _current_simulation_time_;
extern float _current_simulation_dt_;


class Graph : public dataflow::Graph {
public:
	explicit Graph(Session* session);

	void iterate_simulation(float dt);

	Session* session;
};

}
