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
#include "renderer/DeformationRenderer.h"
#include "renderer/GridRenderer.h"
#include "renderer/MeshRenderer.h"
#include "renderer/PointListRenderer.h"
#include "renderer/VectorFieldRenderer.h"
#include "renderer/VolumeRenderer.h"
#include "draw2d/Plotter.h"
#include "draw2d/FunctionPlot.h"
#include "value/NumberListAccumulator.h"
#include "value/RandomNumber.h"
#include "../plugins/PluginManager.h"
#include "draw2d/ListPlot.h"

namespace artemis::graph {

struct NodeClassDescriptor {
	string name;
	base::set<dataflow::NodeCategory> categories;
	std::function<dataflow::Node*(Session*)> f_create;
};

static Array<NodeClassDescriptor> node_class_db;

base::set<dataflow::NodeCategory> category_set(const Array<dataflow::NodeCategory>& categories) {
	base::set<dataflow::NodeCategory> r;
	for (auto c: categories)
		r.add(c);
	return r;
}

template<class T>
void register_node_class(const string& name, const Array<dataflow::NodeCategory>& categories) {
	node_class_db.add({
		name,
		category_set(categories),
		[] (Session*) {
			return new T();
		}
	});
}
template<class T>
void register_node_class_p(const string& name, const Array<dataflow::NodeCategory>& categories) {
	node_class_db.add({
		name,
		category_set(categories),
		[] (Session* s) {
			return new T(s);
		}
	});
}

void init_factory() {
	register_node_class<RegularGrid>("RegularGrid", {dataflow::NodeCategory::Grid});

	register_node_class<SphereMesh>("SphereMesh", {dataflow::NodeCategory::Mesh});
	register_node_class<TeapotMesh>("TeapotMesh", {dataflow::NodeCategory::Mesh});
	register_node_class<IsoSurface>("IsoSurface", {dataflow::NodeCategory::Field, dataflow::NodeCategory::Mesh});

	register_node_class<ScalarField>("ScalarField", {dataflow::NodeCategory::Field});
	register_node_class<VectorField>("VectorField", {dataflow::NodeCategory::Field});
	register_node_class<Gradient>("Gradient", {dataflow::NodeCategory::Field});
	register_node_class<Divergence>("Divergence", {dataflow::NodeCategory::Field});
	register_node_class<Rotation>("Rotation", {dataflow::NodeCategory::Field});
	register_node_class<Laplace>("Laplace", {dataflow::NodeCategory::Field});

	register_node_class_p<Canvas>("Canvas", {dataflow::NodeCategory::Renderer});
	register_node_class_p<GridRenderer>("GridRenderer", {dataflow::NodeCategory::Renderer});
	register_node_class_p<MeshRenderer>("MeshRenderer", {dataflow::NodeCategory::Renderer});
	register_node_class_p<PointListRenderer>("PointListRenderer", {dataflow::NodeCategory::Renderer});
	register_node_class_p<VolumeRenderer>("VolumeRenderer", {dataflow::NodeCategory::Renderer});
	register_node_class_p<VectorFieldRenderer>("VectorFieldRenderer", {dataflow::NodeCategory::Renderer});
	register_node_class_p<DeformationRenderer>("DeformationRenderer", {dataflow::NodeCategory::Renderer});

	register_node_class<FunctionPlot>("FunctionPlot", {dataflow::NodeCategory::Renderer});
	register_node_class<ListPlot>("ListPlot", {dataflow::NodeCategory::Renderer});
	register_node_class_p<Plotter>("Plotter", {dataflow::NodeCategory::Renderer});

	register_node_class<RandomNumber>("RandomNumber", {dataflow::NodeCategory::Field});
	register_node_class<NumberListAccumulator>("NumberListAccumulator", {dataflow::NodeCategory::Field});


	for (const auto& [name, filename] : artemis::PluginManager::plugin_classes) {
		node_class_db.add({
			name,
			{dataflow::NodeCategory::Simulation}, // TODO
			[name=name] (Session*) {
				return (dataflow::Node*)artemis::PluginManager::create_instance(name);
			}
		});
	}
}


Array<string> enumerate_nodes(dataflow::NodeCategory category) {
	base::set<string> _classes;
	for (const auto& d: node_class_db)
		if (d.categories.contains(category))
			_classes.add(d.name);
	return _classes;
}

dataflow::Node* create_node(Session* s, const string& name) {
	for (auto& d: node_class_db)
		if (d.name == name)
			return d.f_create(s);
	msg_error(format("unknown node: %s", name));
	return nullptr;
}

} // graph