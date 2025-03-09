//
// Created by michi on 09.03.25.
//

#ifndef NODEFACTORY_H
#define NODEFACTORY_H

#include <lib/base/base.h>

namespace graph {

class Node;

Array<string> enumerate_nodes();
Node* create_node(const string& name);

} // graph

#endif //NODEFACTORY_H
