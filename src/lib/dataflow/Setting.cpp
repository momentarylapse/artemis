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

template <typename T>
Array<T> any_to_float_list(const Any& a) {
	Array<T> r;
	if (a.is_list())
		for (int i=0; i<a.as_list().num; i++)
			r.add((T)any_to_float(a.as_list()[i]));
	return r;
}

template <typename T>
Any float_list_to_any(const Array<T>& a) {
	Any r = Any::EmptyList;
	for (T x: a)
		r.add(Any(x));
	return r;
}

color any_to_color(const Any& a) {
	const auto list = any_to_float_list<float>(a);
	// rgb-a  :P
	if (list.num >= 4)
		return color(list[3], list[0], list[1], list[2]);
	if (list.num >= 3)
		return color(1, list[0], list[1], list[2]);
	return Black;
}

Any color_to_any(const color& c) {
	Any r;
	r.add(Any(c.r));
	r.add(Any(c.g));
	r.add(Any(c.b));
	r.add(Any(c.a));
	return r;
}

vec3 any_to_vec3(const Any& a) {
	const auto list = any_to_float_list<float>(a);
	if (list.num >= 3)
		return {list[0], list[1], list[2]};
	return vec3::ZERO;
}

Any vec3_to_any(const vec3& v) {
	Any r;
	r.add(Any(v.x));
	r.add(Any(v.y));
	r.add(Any(v.z));
	return r;
}

vec2 any_to_vec2(const Any& a) {
	const auto list = any_to_float_list<float>(a);
	if (list.num >= 2)
		return {list[0], list[1]};
	return vec2::ZERO;
}

Any vec2_to_any(const vec2& v) {
	Any r;
	r.add(Any(v.x));
	r.add(Any(v.y));
	return r;
}

void SettingBase::set_generic(const Any& value) {
	if (class_ == kaba::TypeFloat32) {
		as<float>()->set(any_to_float(value));
	} else if (class_ == kaba::TypeFloat64) {
		as<double>()->set((double)any_to_float(value));
	} else if (class_ == kaba::TypeFloat64List) {
		as<Array<double>>()->set(any_to_float_list<double>(value));
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