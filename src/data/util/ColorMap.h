//
// Created by michi on 03.06.25.
//

#ifndef COLORMAP_H
#define COLORMAP_H

#include <lib/base/base.h>
#include <lib/image/color.h>

namespace artemis::data {

struct ColorMap {
	Array<color> colors;
	Array<float> values;

	float min() const;
	float max() const;
	color get(float f) const;

	static const ColorMap _default;
};

}

#endif //COLORMAP_H
