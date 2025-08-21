//
// Created by michi on 09.03.25.
//

#include "Setting.h"
#include "Node.h"
#include <lib/any/any.h>
#include <lib/any/conversion.h>
#include <lib/kaba/syntax/Class.h>
#include <lib/image/color.h>
#include <lib/math/vec3.h>
#include <lib/os/msg.h>

namespace kaba {
	extern const Class *TypeIntL32ist;
	extern const Class* TypeFloat32List;
	extern const Class* TypeFloat64List;
}

namespace dataflow {

SettingBase::SettingBase(Node* _owner, const string& _name, const kaba::Class* _class_, const string& _options) {
	owner = _owner;
	name = _name;
	class_ = _class_;
	options = _options;
	owner->settings.add(this);
}

void SettingBase::set_generic(const Any& value) {
	if (class_ == kaba::TypeFloat32) {
		as<float>()->set(value.to_f32());
	} else if (class_ == kaba::TypeFloat64) {
		as<double>()->set(value.to_f64());
	} else if (class_ == kaba::TypeFloat64List) {
		as<Array<double>>()->set(any_to_float_list<double>(value));
	} else if (class_ == kaba::TypeInt32) {
		as<int>()->set(value.to_i32());
	} else if (class_ == kaba::TypeString) {
		as<string>()->set(str(value));
	} else if (class_ == kaba::TypeBool) {
		as<bool>()->set(value.to_bool());
	} else if (class_ == kaba::TypeColor) {
		as<color>()->set(any_to_color(value));
	} else if (class_ == kaba::TypeVec3) {
		as<vec3>()->set(any_to_vec3(value));
	} else if (class_ == kaba::TypeVec2) {
		as<vec2>()->set(any_to_vec2(value));
	}
}

Any SettingBase::get_generic() const {
	if (class_ == kaba::TypeFloat32)
		return Any((*as_const<float>())());
	if (class_ == kaba::TypeFloat32List)
		return float_list_to_any<float>((*as_const<Array<float>>())());
	if (class_ == kaba::TypeFloat64)
		return Any((*as_const<double>())());
	if (class_ == kaba::TypeFloat64List)
		return float_list_to_any<double>((*as_const<Array<double>>())());
	if (class_ == kaba::TypeInt32)
		return Any((*as_const<int>())());
	if (class_ == kaba::TypeString)
		return Any((*as_const<string>())());
	if (class_ == kaba::TypeBool)
		return Any((*as_const<bool>())());
	if (class_ == kaba::TypeColor)
		return color_to_any((*as_const<color>())());
	if (class_ == kaba::TypeVec3)
		return vec3_to_any((*as_const<vec3>())());
	if (class_ == kaba::TypeVec2)
		return vec2_to_any((*as_const<vec2>())());
	return {};
}

} // graph