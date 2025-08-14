//
// Created by Michael Ankele on 2025-08-14.
//

#pragma once

#include "base.h"
#include "../grid/RegularGrid.h"


namespace artemis::data {

	struct MultiComponentField {
		MultiComponentField();
		explicit MultiComponentField(const RegularGrid& grid, ScalarType type, SamplingMode sampling_mode, int components);

		ScalarType type;
		RegularGrid grid;
		SamplingMode sampling_mode;
		int components;
		SampledData<float> v32;
		SampledData<double> v64;

		double value(int i, int j, int k, int n) const;
		void set(int i, int j, int k, int n, double v);
		float value32(int i, int j, int k, int n) const;
		void set32(int i, int j, int k, int n, float v);

		void operator+=(const MultiComponentField& o);
		MultiComponentField operator+(const MultiComponentField& o) const;
		void operator-=(const MultiComponentField& o);
		MultiComponentField operator-(const MultiComponentField& o) const;
		void operator*=(float o);
		MultiComponentField operator*(float o) const;

		MultiComponentField componentwise_product(const MultiComponentField& o) const;
	};

}


