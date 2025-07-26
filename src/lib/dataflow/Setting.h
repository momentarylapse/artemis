//
// Created by michi on 09.03.25.
//

#pragma once

#include "Node.h"
#include "Type.h"
#include <lib/base/base.h>
#include <lib/base/xparam.h>

namespace kaba {
	class Class;
}

namespace dataflow {

class Node;


template<class T>
class Setting;

class SettingBase {
public:
	explicit SettingBase(Node* owner, const string& name, const kaba::Class* class_, const string& options);
	void set_generic(const Any& value);
	Any get_generic() const;
	template<class T>
	Setting<T>* as() {
		return static_cast<Setting<T>*>(this);
	}
	template<class T>
	const Setting<T>* as_const() const {
		return static_cast<const Setting<T>*>(this);
	}
	Node* owner;
	string name;
	string options;
	const kaba::Class* class_;

	std::function<void()> on_update;
};

template<class T>
class Setting : public SettingBase {
	friend class SettingBase;
public:
	Setting(Node* owner, const string& name, const T& value, const string& options = "") : SettingBase(owner, name, get_class<T>(), options) {
		this->value = value;
	}
	const T& operator()() const {
		return this->value;
	}
	void set(const typename base::xparam<T>::t value) {
		this->value = value;
		owner->dirty = true;
		if (on_update)
			on_update();
	}
protected:
	T value;
};

template<class T>
class SettingFromSet : public Setting<int> {
public:
	SettingFromSet(Node* owner, const string& name, const T value, const Array<T>& values, const Array<string>& labels) : Setting<int>(owner, name, (int)value) {
		this->options = "set=[";
		for (int i=0; i<values.num; i++) {
			if (i > 0)
				this->options += ",";
			this->options += format("%d:%s", (int)values[i], repr(labels[i]));
		}
		this->options += "]";
	}
	const T operator()() const {
		return (T)this->value;
	}
	void set(const T value) {
		this->set((int)value);
	}
};

} // dataflow

