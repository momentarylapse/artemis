//
// Created by Michael Ankele on 2025-03-10.
//

#include "ScalarField.h"
#include <processing/helper/GlobalThreadPool.h>
#include <lib/os/msg.h>

namespace artemis::data {
	ScalarField::ScalarField(const RegularGrid& g, ScalarType t, SamplingMode s) {
		init(g, t, 1, s);
	}

	ScalarField::ScalarField() : ScalarField(RegularGrid(), ScalarType::None, SamplingMode::PerCell) {}

	double ScalarField::_value(int i, int j, int k) const {
		return value(grid.sample_index(i, j, k, sampling_mode));
	}

	double ScalarField::value(int index) const {
		return Field::value(index, 0);
	}

	void ScalarField::_set(int i, int j, int k, double f) {
		Field::_set(i, j, k, 0, f);
	}

	float ScalarField::_value32(int i, int j, int k) const {
		return (float)_value(i, j, k);
	}

	void ScalarField::_set32(int i, int j, int k, float f) {
		_set(i, j, k, (double)f);
	}

	void ScalarField::set(int index, double f) {
		Field::set(index, 0, f);
	}

	Array<float> ScalarField::as_array32() const {
		Array<float> r;
		if (type == ScalarType::Float32) {
			r.num = n;
			r.data = data.data;
		}
		return r;
	}

	void ScalarField::from_array32(const Array<float>& array) {
		if (type == ScalarType::Float32) {
			if (array.num == n)
				memcpy(data.data, array.data, n*sizeof(float));
		} else if (type == ScalarType::Float64) {
			for (int i=0; i<n; i++)
				Field::set(i, 0, (double)array[i]);
		}
	}

#define CREATE_IMPLACE_SINGLE_OP(OP) \
void ScalarField::operator OP(double o) { \
	if (type == ScalarType::Float32) { \
		processing::pool::run(n, [this, o] (int i) { \
			((float*)data.data)[i] OP o; \
		}, 1000); \
	} else if (type == ScalarType::Float64) { \
		processing::pool::run(n, [this, o] (int i) { \
			((double*)data.data)[i] OP o; \
		}, 1000); \
	} \
}

#define CREATE_IMPLACE_OP(OP) \
void ScalarField::operator OP(const ScalarField& o) { \
	if (o.type != type or sampling_mode != o.sampling_mode) \
		return; \
	if (type == ScalarType::Float32) { \
		processing::pool::run(n, [this, &o] (int i) { \
			((float*)data.data)[i] OP ((float*)o.data.data)[i]; \
		}, 1000); \
	} else if (type == ScalarType::Float64) { \
		processing::pool::run(n, [this, &o] (int i) { \
			((double*)data.data)[i] OP ((double*)o.data.data)[i]; \
		}, 1000); \
	} \
}

#define CREATE_SINGLE_OP(OP) \
ScalarField ScalarField::operator OP(double o) const { \
	auto r = *this; \
	r OP##= o; \
	return r; \
}

#define CREATE_OP(OP) \
ScalarField ScalarField::operator OP(const ScalarField& o) const { \
	auto r = *this; \
	r OP##= o; \
	return r; \
}

CREATE_IMPLACE_SINGLE_OP(=)
CREATE_OP(+)
CREATE_SINGLE_OP(+)
CREATE_IMPLACE_OP(+=)
CREATE_IMPLACE_SINGLE_OP(+=)
CREATE_OP(-)
CREATE_SINGLE_OP(-)
CREATE_IMPLACE_OP(-=)
CREATE_IMPLACE_SINGLE_OP(-=)
CREATE_OP(*)
CREATE_SINGLE_OP(*)
CREATE_IMPLACE_OP(*=)
CREATE_IMPLACE_SINGLE_OP(*=)

/*void ScalarField::operator=(double o) {
	if (type == ScalarType::Float32)
		s_list_assign_single(v32.v, o);
	else if (type == ScalarType::Float64)
		s_list_assign_single(v64.v, o);
}*/

double ScalarField::average() const {
	double sum = 0;
	int n = grid.count(sampling_mode);
	for (int i = 0; i<n; i++)
		sum += value(i);
	return sum;
}

double ScalarField::min() const {
	int n = grid.count(sampling_mode);
	if (n == 0)
		return 0;
	double r = value(0);
	for (int i = 0; i<n; i++)
		r = ::min(r, value(i));
	return r;
}

double ScalarField::max() const {
	int n = grid.count(sampling_mode);
	if (n == 0)
		return 0;
	double r = value(0);
	for (int i = 0; i<n; i++)
		r = ::max(r, value(i));
	return r;
}


}
