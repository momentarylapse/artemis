//
// Created by michi on 03.06.25.
//

#include "ColorMap.h"

namespace artemis::data {

const ColorMap ColorMap::_default = {{color(1, 0,0,1), color(1,1,1,1), color(1,1,0,0)}, {-1,0,1}};

color ColorMap::get(float f) const {
	return Black;
	//return colors[0];
}



}

