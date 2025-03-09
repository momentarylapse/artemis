//
// Created by michi on 09.03.25.
//

#include "Setting.h"

#include "Node.h"

namespace graph {

SettingBase::SettingBase(Node* _owner, const string& _name) {
	owner = _owner;
	name = _name;
	owner->settings.add(this);
}

} // graph