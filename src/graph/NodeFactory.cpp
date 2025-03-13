//
// Created by michi on 09.03.25.
//

#include "NodeFactory.h"
#include "field/Gradient.h"
#include "field/Laplace.h"
#include "field/IsoSurface.h"
#include "field/ScalarField.h"
#include "grid/RegularGrid.h"
#include "mesh/SphereMesh.h"
#include "mesh/TeapotMesh.h"
#include "renderer/GridRenderer.h"
#include "renderer/MeshRenderer.h"
#include "renderer/PointListRenderer.h"
#include "renderer/VectorFieldRenderer.h"
#include "renderer/VolumeRenderer.h"
#include "../plugins/PluginManager.h"

namespace graph {

Array<string> enumerate_nodes() {
	Array<string> a = {
		"SphereMesh",
		"TeapotMesh",
		"IsoSurface",
		"RegularGrid",
		"ScalarField",
		"Gradient",
		"Laplace",
		"GridRenderer",
		"MeshRenderer",
		"PointListRenderer",
		"VolumeRenderer",
		"VectorFieldRenderer",
	};
	for (const auto& [n, f] : artemis::PluginManager::plugin_classes)
		a.add(n);
	return a;
}

Node* create_node(Session* s, const string& name) {
	if (name == "SphereMesh")
		return new SphereMesh();
	if (name == "TeapotMesh")
		return new TeapotMesh();
	if (name == "IsoSurface")
		return new IsoSurface();
	if (name == "RegularGrid")
		return new RegularGrid();
	if (name == "ScalarField")
		return new ScalarField();
	if (name == "Gradient")
		return new Gradient();
	if (name == "Laplace")
		return new Laplace();
	if (name == "GridRenderer")
		return new GridRenderer(s);
	if (name == "MeshRenderer")
		return new MeshRenderer(s);
	if (name == "PointListRenderer")
		return new PointListRenderer(s);
	if (name == "VolumeRenderer")
		return new VolumeRenderer(s);
	if (name == "VectorFieldRenderer")
		return new VectorFieldRenderer(s);
	for (const auto& [n, f] : artemis::PluginManager::plugin_classes)
		if (name == n)
			return (graph::Node*)artemis::PluginManager::create_instance(name);
	return nullptr;
}

} // graph