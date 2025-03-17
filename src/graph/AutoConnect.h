//
// Created by Michael Ankele on 2025-03-17.
//

#ifndef AUTOCONNECT_H
#define AUTOCONNECT_H

namespace graph {

class Graph;
struct CableInfo;

void auto_connect(Graph* g, const CableInfo& c);

} // graph

#endif //AUTOCONNECT_H
