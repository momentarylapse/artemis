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

enum class NodeFlags {
	None = 0,
	Resource = 1,
	Renderer = 2,
	TimeDependent = 4
};
NodeFlags operator|(NodeFlags a, NodeFlags b);
bool operator&(NodeFlags a, NodeFlags b);

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
	NodeFlags flags = NodeFlags::None;

	bool has_necessary_inputs() const;

	Array<SettingBase*> settings;
	Array<InPortBase*> in_ports;
	Array<OutPortBase*> out_ports;
};

class ResourceNode : public Node {
public:
	explicit ResourceNode(const string& name) : Node(name) {
		flags = NodeFlags::Resource;
	}
};

} // graph

#endif //NODE_H
