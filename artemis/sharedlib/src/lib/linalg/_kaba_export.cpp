#include "../base/base.h"
#include "Matrix.h"
#include "../kabaexport/KabaExporter.h"

template<class T>
Array<T> kaba_mul_matrix_vector(const linalg::Matrix<T>& A, const Array<T>& v) {
	return linalg::mul(A, v);
}

template<class T>
linalg::Matrix<T> kaba_mul_matrix_matrix(const linalg::Matrix<T>& A, const linalg::Matrix<T>& B) {
	return linalg::mul(A, B);
}

template<class T>
Array<T> kaba_mul_sparse_matrix_vector(const linalg::SparseMatrix<T>& M, const Array<T>& v) {
	return linalg::mul(M, v);
}

template<class T>
linalg::Matrix<T> kaba_inverse_matrix(const linalg::Matrix<T>& A) {
	return linalg::inverse(A);
}

template<class T>
linalg::Matrix<T> kaba_inverse_sparse_matrix(const linalg::SparseMatrix<T>& A) {
	return linalg::inverse(A);
}

void export_package_linalg(kaba::Exporter* e) {
	using Matrix = linalg::Matrix<float>;
	e->declare_class_size("Matrix", sizeof(Matrix));
	e->declare_class_element("Matrix.cols", &Matrix::cols);
	e->declare_class_element("Matrix.rows", &Matrix::rows);
	e->link_class_func("Matrix.__init__:Matrix", &kaba::generic_init<Matrix>);
	e->link_class_func("Matrix.__init__:Matrix:i32:i32", &kaba::generic_init_ext<Matrix, int, int>);
	e->link_class_func("Matrix.__delete__", &kaba::generic_delete<Matrix>);
	e->link_class_func("Matrix.__assign__", &kaba::generic_assign<Matrix>);
	e->link_class_func("Matrix.clear", &Matrix::clear);
	e->link_class_func("Matrix.get", &Matrix::get);

	using Matrix64 = linalg::Matrix<double>;
	e->declare_class_size("Matrix64", sizeof(Matrix64));
	e->declare_class_element("Matrix64.cols", &Matrix64::cols);
	e->declare_class_element("Matrix64.rows", &Matrix64::rows);
	e->link_class_func("Matrix64.__init__:Matrix64", &kaba::generic_init<Matrix64>);
	e->link_class_func("Matrix64.__init__:Matrix64:i32:i32", &kaba::generic_init_ext<Matrix64, int, int>);
	e->link_class_func("Matrix64.__delete__", &kaba::generic_delete<Matrix64>);
	e->link_class_func("Matrix64.__assign__", &kaba::generic_assign<Matrix64>);
	e->link_class_func("Matrix64.clear", &Matrix64::clear);
	e->link_class_func("Matrix64.get", &Matrix64::get);

	using SparseMatrix = linalg::SparseMatrix<float>;
	e->declare_class_size("SparseMatrix", sizeof(SparseMatrix));
	e->declare_class_element("SparseMatrix.cols", &SparseMatrix::cols);
	e->declare_class_element("SparseMatrix.rows", &SparseMatrix::rows);
	e->link_class_func("SparseMatrix.__init__:SparseMatrix", &kaba::generic_init<SparseMatrix>);
	e->link_class_func("SparseMatrix.__init__:SparseMatrix:i32:i32", &kaba::generic_init_ext<SparseMatrix, int, int>);
	e->link_class_func("SparseMatrix.__delete__", &kaba::generic_delete<SparseMatrix>);
	e->link_class_func("SparseMatrix.__assign__", &kaba::generic_assign<SparseMatrix>);
	e->link_class_func("SparseMatrix.clear", &SparseMatrix::clear);
	e->link_class_func("SparseMatrix.get", &SparseMatrix::get);
	e->link_class_func("SparseMatrix.set", &SparseMatrix::set);
	e->link_class_func("SparseMatrix.to_matrix", &SparseMatrix::to_matrix);

	using SparseMatrix64 = linalg::SparseMatrix<double>;
	e->declare_class_size("SparseMatrix64", sizeof(SparseMatrix64));
	e->declare_class_element("SparseMatrix64.cols", &SparseMatrix64::cols);
	e->declare_class_element("SparseMatrix64.rows", &SparseMatrix64::rows);
	e->link_class_func("SparseMatrix64.__init__:SparseMatrix64", &kaba::generic_init<SparseMatrix64>);
	e->link_class_func("SparseMatrix64.__init__:SparseMatrix64:i32:i32", &kaba::generic_init_ext<SparseMatrix64, int, int>);
	e->link_class_func("SparseMatrix64.__delete__", &kaba::generic_delete<SparseMatrix64>);
	e->link_class_func("SparseMatrix64.__assign__", &kaba::generic_assign<SparseMatrix64>);
	e->link_class_func("SparseMatrix64.clear", &SparseMatrix64::clear);
	e->link_class_func("SparseMatrix64.get", &SparseMatrix64::get);
	e->link_class_func("SparseMatrix64.set", &SparseMatrix64::set);
	e->link_class_func("SparseMatrix64.to_matrix", &SparseMatrix64::to_matrix);

	e->link_func("mul:Matrix:f32[]", &kaba_mul_matrix_vector<float>);
	e->link_func("mul:Matrix:Matrix", &kaba_mul_matrix_matrix<float>);
	e->link_func("mul:SparseMatrix:f32[]", &kaba_mul_sparse_matrix_vector<float>);
	e->link_func("solve:Matrix:f32[]", &linalg::solve<float>);
	e->link_func("inverse:Matrix", &kaba_inverse_matrix<float>);
	e->link_func("inverse:SparseMatrix", &kaba_inverse_sparse_matrix<float>);
	e->link_func("outer_product:SparseMatrix:SparseMatrix", &linalg::outer_product<float>);

	e->link_func("mul:Matrix64:f64[]", &kaba_mul_matrix_vector<double>);
	e->link_func("mul:Matrix64:Matrix64", &kaba_mul_matrix_matrix<double>);
	e->link_func("mul:SparseMatrix64:f64[]", &kaba_mul_sparse_matrix_vector<double>);
	e->link_func("solve:Matrix64:f64[]", &linalg::solve<double>);
	e->link_func("inverse:Matrix64", &kaba_inverse_matrix<double>);
	e->link_func("inverse:SparseMatrix64", &kaba_inverse_sparse_matrix<double>);
	e->link_func("outer_product:SparseMatrix64:SparseMatrix64", &linalg::outer_product<double>);
}


