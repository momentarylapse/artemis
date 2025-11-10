//
// Created by michi on 03.06.25.
//

#pragma once

#include <lib/base/base.h>
#include <lib/image/color.h>

namespace artemis::data {

struct ColorMap {
	Array<color> colors;
	Array<float> values;

	float min() const;
	float max() const;
	color get(float f) const;
	void sort();

	static const ColorMap _default;
	static const ColorMap _default_transparent;
};

}
