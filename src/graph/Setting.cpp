//
// Created by michi on 09.03.25.
//

#include "Setting.h"
#include "Node.h"
#include <lib/any/any.h>
#include <lib/kaba/syntax/Class.h>
#include <lib/image/color.h>
#include <lib/math/vec3.h>
#include <lib/os/msg.h>

namespace graph {

SettingBase::SettingBase(Node* _owner, const string& _name, const kaba::Class* _class_) {
	owner = _owner;
	name = _name;
	class_ = _class_;
	owner->settings.add(this);
}

float any_to_float(const Any& a) {
	if (a.is_float())
		return a.as_float();
	if (a.is_int())
		return (float)a.as_int();
	return 0.0f;
}

int any_to_int(const Any& a) {
	if (a.is_float())
		return (int)a.as_float();
	if (a.is_int())
		return a.as_int();
	return 0;
}

bool any_to_bool(const Any& a) {
	if (a.is_bool())
		return a.as_bool();
	if (a.is_int())
		return a.as_int() == 1;
	return false;
}

Array<float> any_to_float_list(const Any& a) {
	Array<float> r;
	if (a.is_list())
		for (int i=0; i<a.as_list().num; i++)
			r.add(any_to_float(a.as_list()[i]));
	return r;
}

color any_to_color(const Any& a) {
	const auto list = any_to_float_list(a);
	// rgb-a  :P
	if (list.num >= 4)
		return color(list[3], list[0], list[1], list[2]);
	if (list.num >= 3)
		return color(1, list[0], list[1], list[2]);
	return Black;
}

vec3 any_to_vec3(const Any& a) {
	const auto list = any_to_float_list(a);
	if (list.num >= 3)
		return {list[0], list[1], list[2]};
	return vec3::ZERO;
}

vec2 any_to_vec2(const Any& a) {
	const auto list = any_to_float_list(a);
	if (list.num >= 2)
		return {list[0], list[1]};
	return vec2::ZERO;
}

void SettingBase::set_generic(const Any& value) {
	if (class_ == kaba::TypeFloat32) {
		as<float>()->set(any_to_float(value));
	} else if (class_ == kaba::TypeInt32) {
		as<int>()->set(any_to_int(value));
	} else if (class_ == kaba::TypeString) {
		as<string>()->set(str(value));
	} else if (class_ == kaba::TypeBool) {
		as<bool>()->set(any_to_bool(value));
	} else if (class_ == kaba::TypeColor) {
		as<color>()->set(any_to_color(value));
	} else if (class_ == kaba::TypeVec3) {
		as<vec3>()->set(any_to_vec3(value));
	} else if (class_ == kaba::TypeVec2) {
		as<vec2>()->set(any_to_vec2(value));
	}
}

} // graph