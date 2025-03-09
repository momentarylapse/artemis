//
// Created by michi on 09.03.25.
//

#ifndef SETTING_H
#define SETTING_H

#include "Node.h"
#include <lib/base/base.h>
#include <plugins/PluginManager.h>

namespace kaba {
	class Class;
}

namespace graph {

class Node;

class SettingBase {
public:
	explicit SettingBase(Node* owner, const string& name, const kaba::Class* class_);
	Node* owner;
	string name;
	const kaba::Class* class_;
};

template<class T>
class Setting : public SettingBase {
public:
	Setting(Node* owner, const string& name, const T& value) : SettingBase(owner, name, artemis::get_class<T>()) {
		this->value = value;
	}
	const T& operator()() const {
		return this->value;
	}
	void set(const T& value) {
		this->value = value;
		owner->dirty = true;
	}
private:
	T value;
};

} // graph

#endif //SETTING_H
