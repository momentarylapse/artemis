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


template<class T>
class Setting;

class SettingBase {
public:
	explicit SettingBase(Node* owner, const string& name, const kaba::Class* class_);
	void set_generic(const Any& value);
	template<class T>
	Setting<T>* as() {
		return static_cast<Setting<T>*>(this);
	}
	Node* owner;
	string name;
	const kaba::Class* class_;

	std::function<void()> on_update;
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
		if (on_update)
			on_update();
	}
private:
	T value;
};

} // graph

#endif //SETTING_H
