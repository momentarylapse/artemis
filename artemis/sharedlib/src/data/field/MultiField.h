//
// Created by Michael Ankele on 2025-08-14.
//

#pragma once

#include "Field.h"
#include "../grid/RegularGrid.h"


namespace artemis::data {

	struct ScalarField;

	struct MultiField : Field {
		MultiField();
		explicit MultiField(const RegularGrid& grid, ScalarType type, SamplingMode sampling_mode, int components);
		MultiField(const MultiField& other);
		~MultiField();

		Array<double> values(int index) const;

		void operator=(const MultiField& o);
		void operator+=(const MultiField& o);
		MultiField operator+(const MultiField& o) const;
		void operator-=(const MultiField& o);
		MultiField operator-(const MultiField& o) const;
		void operator*=(double o);
		MultiField operator*(double o) const;

		ScalarField get_component(int i) const;
		MultiField componentwise_product(const MultiField& o) const;
	};

}


