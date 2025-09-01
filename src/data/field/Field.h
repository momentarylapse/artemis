//
// Created by michi on 9/1/25.
//

#pragma once

#include "base.h"

namespace artemis::data {

	struct Field {
		RegularGrid grid;
		ScalarType type;
		SamplingMode sampling_mode;
		bytes data;
		int n = 0;
		int components = 0;
		int stride = 0; // bytes

		void init(const RegularGrid& _grid, ScalarType _type, int _components, SamplingMode mode);
		void* at(int index) {
			return &data[stride * index];
		}
		const void* at(int index) const {
			return (char*)data.data + (stride * index);
		}
		void* at(int i, int j, int k) {
			return at(grid.sample_index(i, j, k, sampling_mode));
		}
		const void* at(int i, int j, int k) const {
			return at(grid.sample_index(i, j, k, sampling_mode));
		}


		double value(int index, int n) const;
		void set(int index, int n, double v);


		double _value(int i, int j, int k, int n) const;
		void _set(int i, int j, int k, int n, double vv);

		//void operator=(const Field& o);

		/*void cwise_product(const SampledData<S>& a, const SampledData<S>& b) {
			//if (a.components != b.components or a.components != components)
			//	return;
			if (a.v.num != v.num or b.v.num != v.num)
				return;
			for (int i=0; i<v.num; i++)
				v[i] = a.v[i] * b.v[i];
		}*/
};

}
