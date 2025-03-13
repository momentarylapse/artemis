//
// Created by michi on 09.03.25.
//

#ifndef NODE_H
#define NODE_H

#include <lib/base/base.h>
#include <lib/math/vec2.h>
#include <lib/pattern/Observable.h>

class Any;

namespace xhui {
	class Panel;
}

namespace graph {

class SettingBase;
class InPortBase;
class OutPortBase;

enum class NodeCategory {
	Field,
	Grid,
	Mesh,
	Renderer,
	Simulation
};

class Node : public obs::Node<VirtualBase> {
public:
	explicit Node(const string& name);
	~Node() override = default;

	virtual void process() {}

	virtual xhui::Panel* create_panel();

	void set(const string& key, const Any& value);

	string name;
	vec2 pos;
	bool dirty = true;
	bool is_resource_node = false;
	bool is_renderer = false;

	Array<SettingBase*> settings;
	Array<InPortBase*> in_ports;
	Array<OutPortBase*> out_ports;
};

class ResourceNode : public Node {
public:
	explicit ResourceNode(const string& name) : Node(name) {
		is_resource_node = true;
	}
};

} // graph

#endif //NODE_H
