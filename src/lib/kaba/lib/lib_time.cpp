#include "../../os/date.h"
#include "../../os/time.h"
#include "../kaba.h"
#include "lib.h"


namespace kaba {


extern const Class *TypeDate;
const Class *TypeTimer;

void SIAddPackageTime(Context *c) {
	add_internal_package(c, "time");

	TypeDate = add_type("Date", sizeof(Date));
	TypeTimer = add_type("Timer", sizeof(os::Timer));


	add_class(TypeDate);
		class_add_element("time", TypeInt64, &Date::time);
		class_add_func("format", TypeString, &Date::format, Flags::Pure);
			func_add_param("f", TypeString);
		class_add_func(Identifier::func::Str, TypeString, &Date::str, Flags::Pure);
		class_add_func(Identifier::func::Assign, TypeVoid, &Date::__assign__, Flags::Mutable);
			func_add_param("o", TypeDate);


	add_class(TypeTimer);
		class_add_func(Identifier::func::Init, TypeVoid, &os::Timer::reset, Flags::Mutable);
		class_add_func("get", TypeFloat32, &os::Timer::get, Flags::Mutable);
		class_add_func("reset", TypeVoid, &os::Timer::reset, Flags::Mutable);
		class_add_func("peek", TypeFloat32, &os::Timer::peek);


	add_func("sleep", TypeVoid, &os::sleep, Flags::Static);
		func_add_param("duration", TypeFloat32);
	add_func("now", TypeDate, &Date::now, Flags::Static);
}

};
