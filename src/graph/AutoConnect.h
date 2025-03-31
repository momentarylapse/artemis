//
// Created by Michael Ankele on 2025-03-17.
//

#ifndef AUTOCONNECT_H
#define AUTOCONNECT_H

namespace dataflow {
	struct CableInfo;
}

namespace artemis::graph {

class Graph;

void auto_connect(Graph* g, const dataflow::CableInfo& c);

} // graph

#endif //AUTOCONNECT_H
