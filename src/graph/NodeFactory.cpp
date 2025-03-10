//
// Created by michi on 09.03.25.
//

#include "NodeFactory.h"

#include "field/ScalarField.h"
#include "grid/RegularGrid.h"
#include "mesh/SphereMesh.h"
#include "mesh/TeapotMesh.h"
#include "renderer/GridRenderer.h"
#include "renderer/MeshRenderer.h"
#include "renderer/PointListRenderer.h"
#include "renderer/VolumeRenderer.h"

namespace graph {

Array<string> enumerate_nodes() {
	return {
		"SphereMesh",
		"TeapotMesh",
		"RegularGrid",
		"ScalarField",
		"GridRenderer",
		"MeshRenderer",
		"PointListRenderer",
		"VolumeRenderer",
	};
}
Node* create_node(Session* s, const string& name) {
	if (name == "SphereMesh")
		return new SphereMesh();
	if (name == "TeapotMesh")
		return new TeapotMesh();
	if (name == "RegularGrid")
		return new RegularGrid();
	if (name == "ScalarField")
		return new ScalarField();
	if (name == "GridRenderer")
		return new GridRenderer(s);
	if (name == "MeshRenderer")
		return new MeshRenderer(s);
	if (name == "PointListRenderer")
		return new PointListRenderer(s);
	if (name == "VolumeRenderer")
		return new VolumeRenderer(s);
	return nullptr;
}

} // graph