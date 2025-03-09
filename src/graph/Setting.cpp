//
// Created by michi on 09.03.25.
//

#include "Setting.h"

#include "Node.h"

namespace graph {

SettingBase::SettingBase(Node* _owner, const string& _name, const kaba::Class* _class_) {
	owner = _owner;
	name = _name;
	class_ = _class_;
	owner->settings.add(this);
}

} // graph