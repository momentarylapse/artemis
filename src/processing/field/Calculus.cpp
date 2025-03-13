//
// Created by Michael Ankele on 2025-03-13.
//

#include "Calculus.h"
#include <data/field/ScalarField.h>
#include <data/field/VectorField.h>

namespace artemis::processing {

data::VectorField gradient(const data::ScalarField& f) {
	data::VectorField v(f.grid);

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

data::ScalarField laplace(const data::ScalarField& f) {
	data::ScalarField v(f.grid);

	for (int i=1; i<f.grid.nx; i++)
		for (int j=1; j<f.grid.ny; j++)
			for (int k=1; k<f.grid.nz; k++) {
				float f0 = f.value(i, j, k);
				float fmx = f.value(i-1, j, k);
				float fx = f.value(i+1, j, k);
				float fmy = f.value(i, j-1, k);
				float fy = f.value(i, j+1, k);
				float fmz = f.value(i, j, k-1);
				float fz = f.value(i, j, k+1);
				v.set(i, j, k, fmx+fx + fmy+fy + fmz+fz - 6*f0);
			}

	return v;
}

}

