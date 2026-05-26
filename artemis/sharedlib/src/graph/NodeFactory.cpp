//
// Created by michi on 09.03.25.
//

#include "NodeFactory.h"
#include "field/ScalarField.h"
#include "field/VectorField.h"
#include "field/MultiComponentField.h"
#include "canvas/Canvas.h"
#include "../plugins/PluginManager.h"
#include <lib/os/msg.h>

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

void init_factory() {
	register_node_class<ScalarField>("ScalarField", {dataflow::NodeCategory::Field});
	register_node_class<VectorField>("VectorField", {dataflow::NodeCategory::Field});
	register_node_class<MultiComponentField>("MultiComponentField", {dataflow::NodeCategory::Field});

	register_node_class<Canvas>("Canvas", {dataflow::NodeCategory::Renderer});

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
	for (const auto& class_name : artemis::PluginManager::template_classes) {
		node_class_db.add({
			class_name,
			{dataflow::NodeCategory::Meta},
			[class_name=class_name] (Session*, const string& name) {
				msg_error("TOTO template instance");
				return nullptr;
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