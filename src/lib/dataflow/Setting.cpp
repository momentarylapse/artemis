//
// Created by michi on 09.03.25.
//

#include "Setting.h"
#include "Node.h"
#include <lib/any/any.h>
#include <lib/kaba/dynamic/dynamic.h>


namespace dataflow {

SettingBase::SettingBase(Node* _owner, const string& _name, const kaba::Class* _type, void* p, const string& _options) {
	owner = _owner;
	name = _name;
	type = _type;
	generic_value_pointer = p;
	options = _options;
	owner->settings.add(this);
}

void SettingBase::set_generic(const Any& value) {
	kaba::unwrap_any(value, generic_value_pointer, type);
	owner->dirty = true;
	if (on_update)
		on_update();
}

Any SettingBase::get_generic() const {
	return kaba::dynify(generic_value_pointer, type);
}

void SettingBase::generic_set(void *p) {
	generic_value_pointer = p;
	owner->dirty = true;
	if (on_update)
		on_update();
}

} // graph