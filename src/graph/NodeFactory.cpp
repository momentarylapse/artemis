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
#include "field/VectorFieldComponent.h"
#include "field/VectorFieldLength.h"
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
#include "list/ListToVectors.h"
#include "list/List.h"
#include "list/ListAccumulator.h"
#include "value/RandomNumber.h"
#include "../plugins/PluginManager.h"
#include "draw2d/ListPlot.h"

namespace artemis::graph {

struct NodeClassDescriptor {
	string name;
	base::set<dataflow::NodeCategory> categories;
	std::function<dataflow::Node*(Session*,const string&)> f_create;
};

static Array<NodeClassDescriptor> node_class_db;

base::set<dataflow::NodeCategory> category_set(const Array<dataflow::NodeCategory>& categories) {
	base::set<dataflow::NodeCategory> r;
	for (auto c: categories)
		r.add(c);
	return r;
}

template<class T>
void register_node_class(const string& class_name, const Array<dataflow::NodeCategory>& categories) {
	node_class_db.add({
		class_name,
		category_set(categories),
		[] (Session*, const string& name) {
			auto node = new T();
			node->name = name;
			return node;
		}
	});
}
template<class T>
void register_node_class_p(const string& class_name, const Array<dataflow::NodeCategory>& categories) {
	node_class_db.add({
		class_name,
		category_set(categories),
		[] (Session* s, const string& name) {
			auto node = new T(s);
			node->name = name;
			return node;
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
	register_node_class<VectorFieldComponent>("VectorFieldComponent", {dataflow::NodeCategory::Field});
	register_node_class<VectorFieldLength>("VectorFieldLength", {dataflow::NodeCategory::Field});

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
	register_node_class<List>("List", {dataflow::NodeCategory::Field});
	register_node_class<ListAccumulator>("ListAccumulator", {dataflow::NodeCategory::Field});
	register_node_class<ListToVectors>("ListToVectors", {dataflow::NodeCategory::Field});


	for (const auto& [class_name, filename] : artemis::PluginManager::plugin_classes) {
		node_class_db.add({
			class_name,
			{dataflow::NodeCategory::Simulation}, // TODO
			[class_name=class_name] (Session*, const string& name) {
				auto n = (dataflow::Node*)artemis::PluginManager::create_instance(class_name);
				if (n)
					n->name = name;
				return n;
			}
		});
	}
}


Array<string> enumerate_nodes(dataflow::NodeCategory category) {
	base::set<string> _classes;
	for (const auto& d: node_class_db)
		if (d.categories.contains(category) or category == dataflow::NodeCategory::None)
			_classes.add(d.name);
	return _classes;
}

dataflow::Node* create_node(Session* s, const string& name) {
	const string class_name = name.explode(":")[0];
	for (auto& d: node_class_db)
		if (d.name == class_name)
			return d.f_create(s, name);
	msg_error(format("unknown node class: %s", class_name));
	return nullptr;
}

} // graph