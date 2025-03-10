//
// Created by michi on 09.03.25.
//

#include "NodeFactory.h"
#include "mesh/SphereMesh.h"
#include "mesh/TeapotMesh.h"
#include "renderer/MeshRenderer.h"

namespace graph {

Array<string> enumerate_nodes() {
	return {
		"SphereMesh",
		"TeapotMesh",
		"MeshRenderer"
	};
}
Node* create_node(Session* s, const string& name) {
	if (name == "SphereMesh")
		return new SphereMesh();
	if (name == "TeapotMesh")
		return new TeapotMesh();
	if (name == "MeshRenderer")
		return new MeshRenderer(s);
	return nullptr;
}

} // graph