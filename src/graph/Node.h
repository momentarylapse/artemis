//
// Created by michi on 09.03.25.
//

#ifndef NODE_H
#define NODE_H

#include <lib/base/base.h>
#include <lib/math/vec2.h>
#include <lib/pattern/Observable.h>

class Any;

namespace graph {

class SettingBase;
class InPortBase;
class OutPortBase;

class Node : public obs::Node<VirtualBase> {
public:
	explicit Node(const string& name);

	virtual void process() {}

	void set(const string& key, const Any& value);

	string name;
	vec2 pos;
	bool dirty = true;

	Array<SettingBase*> settings;
	Array<InPortBase*> in_ports;
	Array<OutPortBase*> out_ports;
};

} // graph

#endif //NODE_H
