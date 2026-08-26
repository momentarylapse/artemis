//
// Created by Michael Ankele on 2025-03-23.
//

#pragma once

#include <lib/base/base.h>

namespace artemis::data {

struct RegularGrid;

enum class ScalarType {
	None,
	Float32,
	Float64
};

inline int scalar_size(ScalarType type) {
	switch (type) {
		case ScalarType::Float32:
			return 4;
		case ScalarType::Float64:
			return 8;
		default:
			return 1;
	}
}

// also basis functions
enum class SamplingMode {
	PerCell,
	PerVertex,
	PerEdge
};

}

