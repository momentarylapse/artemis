//
// Created by michi on 8/15/25.
//

#pragma once

#include "base.h"
#include "../grid/RegularGrid.h"
#include <lib/linalg/Matrix.h>

namespace artemis::data {

enum class BasisType {
	P1 // TODO... piecewise linear....
};

struct BasisFields {
	int grid_hash;
	linalg::SparseMatrix<float> phi_phi;
	linalg::Matrix<float> phi_phi_inv;
	linalg::SparseMatrix<float> phi_dx_phi;
	linalg::SparseMatrix<float> phi_dy_phi;
	linalg::SparseMatrix<float> phi_dz_phi;
	linalg::SparseMatrix<float> phi_dx_dx_phi;
	linalg::SparseMatrix<float> phi_dx_dy_phi;
	linalg::SparseMatrix<float> phi_dx_dz_phi;
	linalg::SparseMatrix<float> phi_dy_dy_phi;
	linalg::SparseMatrix<float> phi_dy_dz_phi;
	linalg::SparseMatrix<float> phi_dz_dz_phi;
	linalg::Matrix<float> dx_dx;
	linalg::Matrix<float> dx_dy;
	linalg::Matrix<float> dx_dz;
	linalg::Matrix<float> dy_dy;
	linalg::Matrix<float> dy_dz;
	linalg::Matrix<float> dz_dz;
};

BasisFields create_basis_fields(const RegularGrid& grid);
const BasisFields& get_basis_fields(const RegularGrid& grid);

}