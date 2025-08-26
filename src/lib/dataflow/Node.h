//
// Created by michi on 09.03.25.
//

#pragma once

#include <lib/base/base.h>
#include <lib/math/vec2.h>
#include <lib/pattern/Observable.h>

class Any;

namespace xhui {
	class Panel;
}

namespace dataflow {

class SettingBase;
class InPortBase;
class OutPortBase;
class Graph;

enum class NodeCategory {
	None,
	Field,
	Grid,
	Mesh,
	Renderer,
	Simulation,
	Meta
};

enum class NodeFlags {
	None = 0,
	Resource = 1,
	Renderer = 2,
	TimeDependent = 4,
	Canvas = 8,
	Meta = 16
};
NodeFlags operator|(NodeFlags a, NodeFlags b);
bool operator&(NodeFlags a, NodeFlags b);

enum class NodeState {
	Uninitialized,
	Dirty,
	Complete
};

class Node : public obs::Node<VirtualBase> {
public:
	explicit Node(const string& name);
	~Node() override;

	virtual void additional_init() {}

	virtual void on_process() {}
	void process();

	virtual xhui::Panel* create_panel();

	void on_settings_changed(SettingBase* s);
	void on_input_changed(InPortBase* p);

	void set(const string& key, const Any& value);
	Any get(const string& key) const;

	Graph* graph = nullptr;
	string name;
	vec2 pos;
	int channel;
	NodeState state = NodeState::Uninitialized;
	NodeFlags flags = NodeFlags::None;

	bool has_necessary_inputs() const;

	Array<SettingBase*> settings;
	Array<InPortBase*> in_ports;
	Array<OutPortBase*> out_ports;
};

} // dataflow

