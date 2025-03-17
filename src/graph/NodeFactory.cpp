//
// Created by michi on 09.03.25.
//

#include "NodeFactory.h"

#include <lib/os/msg.h>

#include "field/Gradient.h"
#include "field/Laplace.h"
#include "field/Divergence.h"
#include "field/Rotation.h"
#include "field/IsoSurface.h"
#include "field/ScalarField.h"
#include "field/VectorField.h"
#include "grid/RegularGrid.h"
#include "mesh/SphereMesh.h"
#include "mesh/TeapotMesh.h"
#include "renderer/Canvas.h"
#include "renderer/GridRenderer.h"
#include "renderer/MeshRenderer.h"
#include "renderer/PointListRenderer.h"
#include "renderer/VectorFieldRenderer.h"
#include "renderer/VolumeRenderer.h"
#include "draw2d/Plot.h"
#include "../plugins/PluginManager.h"

namespace graph {

struct NodeClassDescriptor {
	string name;
	base::set<NodeCategory> categories;
	std::function<Node*(Session*)> f_create;
};

static Array<NodeClassDescriptor> node_class_db;

base::set<NodeCategory> category_set(const Array<NodeCategory>& categories) {
	base::set<NodeCategory> r;
	for (auto c: categories)
		r.add(c);
	return r;
}

template<class T>
void register_node_class(const string& name, const Array<NodeCategory>& categories) {
	node_class_db.add({
		name,
		category_set(categories),
		[] (Session*) {
			return new T();
		}
	});
}
template<class T>
void register_node_class_p(const string& name, const Array<NodeCategory>& categories) {
	node_class_db.add({
		name,
		category_set(categories),
		[] (Session* s) {
			return new T(s);
		}
	});
}

void init_factory() {
	register_node_class<RegularGrid>("RegularGrid", {NodeCategory::Grid});

	register_node_class<SphereMesh>("SphereMesh", {NodeCategory::Mesh});
	register_node_class<TeapotMesh>("TeapotMesh", {NodeCategory::Mesh});
	register_node_class<IsoSurface>("IsoSurface", {NodeCategory::Field, NodeCategory::Mesh});

	register_node_class<ScalarField>("ScalarField", {NodeCategory::Field});
	register_node_class<VectorField>("VectorField", {NodeCategory::Field});
	register_node_class<Gradient>("Gradient", {NodeCategory::Field});
	register_node_class<Divergence>("Divergence", {NodeCategory::Field});
	register_node_class<Rotation>("Rotation", {NodeCategory::Field});
	register_node_class<Laplace>("Laplace", {NodeCategory::Field});

	register_node_class_p<Canvas>("Canvas", {NodeCategory::Renderer});
	register_node_class_p<GridRenderer>("GridRenderer", {NodeCategory::Renderer});
	register_node_class_p<MeshRenderer>("MeshRenderer", {NodeCategory::Renderer});
	register_node_class_p<PointListRenderer>("PointListRenderer", {NodeCategory::Renderer});
	register_node_class_p<VolumeRenderer>("VolumeRenderer", {NodeCategory::Renderer});
	register_node_class_p<VectorFieldRenderer>("VectorFieldRenderer", {NodeCategory::Renderer});

	register_node_class_p<Plot>("Plot", {NodeCategory::Renderer});


	for (const auto& [name, filename] : artemis::PluginManager::plugin_classes) {
		node_class_db.add({
			name,
			{NodeCategory::Simulation}, // TODO
			[name=name] (Session*) {
				return (Node*)artemis::PluginManager::create_instance(name);
			}
		});
	}
}


Array<string> enumerate_nodes(NodeCategory category) {
	base::set<string> _classes;
	for (const auto& d: node_class_db)
		if (d.categories.contains(category))
			_classes.add(d.name);
	return _classes;
}

Node* create_node(Session* s, const string& name) {
	for (auto& d: node_class_db)
		if (d.name == name)
			return d.f_create(s);
	msg_error(format("unknown node: %s", name));
	return nullptr;
}

} // graph