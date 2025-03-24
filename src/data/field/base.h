//
// Created by Michael Ankele on 2025-03-23.
//

#pragma once

#include <lib/base/base.h>
#include "../grid/RegularGrid.h"

namespace artemis::data {

struct RegularGrid;

enum class ScalarType {
	None,
	Float32,
	Float64
};

// also basis functions
enum class SamplingMode {
	PerCell,
	PerVertex
};

template<class T, class S>
struct SampledData {
	using ItemType = T;
	using ScalarType = S;
	Array<T> v;
	void init(const RegularGrid& grid, SamplingMode mode) {
		if (mode == SamplingMode::PerCell)
			v.resize(grid.cell_count());
		else if (mode == SamplingMode::PerVertex)
			v.resize(grid.vertex_count());
	}
	T& at(const RegularGrid& grid, SamplingMode sampling_mode, int i, int j, int k) {
		if (sampling_mode == SamplingMode::PerCell)
			return v[grid.cell_index(i, j, k)];
		//	if (sampling_mode == SamplingMode::PerVertex)
		return v[grid.vertex_index(i, j, k)];
	}
	const T& at(const RegularGrid& grid, SamplingMode sampling_mode, int i, int j, int k) const {
		if (sampling_mode == SamplingMode::PerCell)
			return v[grid.cell_index(i, j, k)];
		//	if (sampling_mode == SamplingMode::PerVertex)
		return v[grid.vertex_index(i, j, k)];
	}
};

}

