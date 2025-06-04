//
// Created by michi on 03.06.25.
//

#include "ColorMap.h"

namespace artemis::data {

const ColorMap ColorMap::_default = {{color(1, 0,0,1), color(1,1,1,1), color(1,1,0,0)}, {-1,0,1}};

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
	for (int i=0; i<values.num-1; i++)
		if (f >= values[i] and f <= values[i+1])
			return color::interpolate(colors[i], colors[i+1], (f - values[i]) / (values[i+1] - values[i]));
	return Black;
}

}

