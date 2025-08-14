//
// Created by Michael Ankele on 2025-03-13.
//

#pragma once

namespace artemis::data {
struct ScalarField;
struct VectorField;
}

namespace artemis::processing {
data::VectorField gradient(const data::ScalarField& f);
data::ScalarField laplace(const data::ScalarField& f);
data::ScalarField divergence(const data::VectorField& f);
data::VectorField rotation_fw(const data::VectorField& f);
data::VectorField rotation_bw(const data::VectorField& f);

data::ScalarField hessian_x(const data::ScalarField& f, int i, int j);
}
