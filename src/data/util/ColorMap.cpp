//
// Created by michi on 03.06.25.
//

#include "ColorMap.h"

namespace artemis::data {

const ColorMap ColorMap::_default = {{color(1, 0,0,1), color(1,1,1,1), color(1,1,0,0)}, {-1,0,1}};
const ColorMap ColorMap::_default_transparent = {{color(0.5f, 0,0,1), color(0,1,1,1), color(0.5f,1,0,0)}, {-1,0,1}};

float ColorMap::min() const {
	if (values.num >= 1)
		return values[0];
	return 0;
}

float ColorMap::max() const {
	if (values.num >= 2)
		return values.back();
	return min() + 1;
}

color ColorMap::get(float f) const {
	if (values.num > 0 and f < values[0])
		return colors[0];
	for (int i=0; i<values.num-1; i++)
		if (f >= values[i] and f <= values[i+1])
			return color::interpolate(colors[i], colors[i+1], (f - values[i]) / (values[i+1] - values[i]));
	if (values.num >= 1)
		return colors[1];
	return Black;
}

}

