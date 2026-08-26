//
// Created by Michael Ankele on 2025-03-10.
//

#pragma once

#include "base.h"
#include "Field.h"
#include "../grid/RegularGrid.h"

namespace artemis::data {

struct ScalarField : Field {
	ScalarField();
	explicit ScalarField(const RegularGrid& grid, ScalarType type, SamplingMode sampling_mode);

	double value(int index) const;
	void set(int index, double f);
	double _value(int i, int j, int k) const;
	void _set(int i, int j, int k, double f);
	float _value32(int i, int j, int k) const;
	void _set32(int i, int j, int k, float f);

	double average() const;
	double min() const;
	double max() const;

	Array<float> as_array32() const;
	void from_array32(const Array<float>& array);

	void operator=(double o);
	void operator+=(const ScalarField& o);
	void operator+=(double o);
	ScalarField operator+(const ScalarField& o) const;
	ScalarField operator+(double o) const;
	void operator-=(const ScalarField& o);
	void operator-=(double o);
	ScalarField operator-(const ScalarField& o) const;
	ScalarField operator-(double o) const;
	void operator*=(const ScalarField& o);
	void operator*=(double o);
	ScalarField operator*(const ScalarField& o) const;
	ScalarField operator*(double o) const;
};

}

