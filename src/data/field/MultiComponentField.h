//
// Created by Michael Ankele on 2025-08-14.
//

#pragma once

#include "Field.h"
#include "../grid/RegularGrid.h"


namespace artemis::data {

	struct MultiComponentField : Field {
		MultiComponentField();
		explicit MultiComponentField(const RegularGrid& grid, ScalarType type, SamplingMode sampling_mode, int components);

		Array<double> values(int index) const;

		void operator+=(const MultiComponentField& o);
		MultiComponentField operator+(const MultiComponentField& o) const;
		void operator-=(const MultiComponentField& o);
		MultiComponentField operator-(const MultiComponentField& o) const;
		void operator*=(double o);
		MultiComponentField operator*(double o) const;

		MultiComponentField componentwise_product(const MultiComponentField& o) const;
	};

}


