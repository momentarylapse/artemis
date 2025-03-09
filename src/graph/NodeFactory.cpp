//
// Created by michi on 09.03.25.
//

#include "NodeFactory.h"

#include "mesh/TeapotMesh.h"
#include "renderer/MeshRenderer.h"

namespace graph {

Array<string> enumerate_nodes() {
	return {
		"TeapotMesh",
		"MeshRenderer"
	};
}
Node* create_node(const string& name) {
	if (name == "TeapotMesh")
		return new TeapotMesh();
	if (name == "MeshRenderer")
		return new MeshRenderer();
	return nullptr;
}

} // graph