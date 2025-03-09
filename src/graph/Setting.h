//
// Created by michi on 09.03.25.
//

#ifndef SETTING_H
#define SETTING_H

#include <lib/base/base.h>

namespace graph {

class Node;

class SettingBase {
public:
	explicit SettingBase(Node* owner, const string& name);
	Node* owner;
	string name;
};

template<class T>
class Setting : public SettingBase {
public:
	Setting(Node* owner, const string& name, const T& value) : SettingBase(owner, name) {
		this->value = value;
	}
	const T& operator()() const {
		return this->value;
	}
	T value;
};

} // graph

#endif //SETTING_H
