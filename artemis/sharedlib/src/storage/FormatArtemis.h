/*
 * FormatArtemis.h
 *
 *  Created on: 03.06.2025
 *      Author: michi
 */

#pragma once

#include <graph/Graph.h>

namespace artemis {
	namespace graph {
		class DataGraph;
	}

	void load_artemis_file(graph::DataGraph *data, const Path &filename);
	void save_artemis_file(graph::DataGraph *data, const Path &filename);
}
