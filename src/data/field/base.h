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

template<class S>
struct SampledData {
	using ScalarType = S;
	Array<S> v;
	int components;
	void init(const RegularGrid& grid, int _components, SamplingMode mode) {
		components = _components;
		if (mode == SamplingMode::PerCell)
			v.resize(components * grid.cell_count());
		else if (mode == SamplingMode::PerVertex)
			v.resize(components * grid.vertex_count());
	}
	S* _at(int index) {
		return &v[components * index];
	}
	const S* _at(int index) const {
		return &v[components * index];
	}
	S* at(const RegularGrid& grid, SamplingMode sampling_mode, int i, int j, int k) {
		if (sampling_mode == SamplingMode::PerCell)
			return _at(grid.cell_index(i, j, k));
		//	if (sampling_mode == SamplingMode::PerVertex)
		return _at(grid.vertex_index(i, j, k));
	}
	const S* at(const RegularGrid& grid, SamplingMode sampling_mode, int i, int j, int k) const {
		if (sampling_mode == SamplingMode::PerCell)
			return _at(grid.cell_index(i, j, k));
		//	if (sampling_mode == SamplingMode::PerVertex)
		return _at(grid.vertex_index(i, j, k));
	}
	void cwise_product(const SampledData<S>& a, const SampledData<S>& b) {
		//if (a.components != b.components or a.components != components)
		//	return;
		if (a.v.num != v.num or b.v.num != v.num)
			return;
		for (int i=0; i<v.num; i++)
			v[i] = a.v[i] * b.v[i];
	}
};

}

