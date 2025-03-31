//
// Created by Michael Ankele on 2025-03-10.
//

#include "RendererNode.h"

namespace artemis::graph {

RendererNode::RendererNode(Session* s, const string& name) : Node(name) {
	session = s;
	flags = dataflow::NodeFlags::Renderer;
}

} // graph