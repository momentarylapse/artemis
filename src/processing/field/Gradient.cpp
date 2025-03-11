//
// Created by Michael Ankele on 2025-03-11.
//

#include "Gradient.h"
#include <data/field/ScalarField.h>
#include <data/field/VectorField.h>

namespace artemis::processing {

data::VectorField gradient(const data::ScalarField& f) {
	artemis::data::VectorField v(f.grid);

	for (int i=0; i<f.grid.nx; i++)
		for (int j=0; j<f.grid.ny; j++)
			for (int k=0; k<f.grid.nz; k++) {
				float f0 = f.value(i, j, k);
				float fx = f.value(i+1, j, k);
				float fy = f.value(i, j+1, k);
				float fz = f.value(i, j, k+1);
				v.set(i, j, k, vec3(fx - f0, fy - f0, fz - f0));
			}

	return v;
}

}
