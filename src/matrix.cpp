/******************************************************************************
 * @details This is a file containing declarations of [Matrix].
 * 
 * @author     Adam Matthew Blakey
 * @date       2020/01/02
 ******************************************************************************/
#ifndef CLASS_SRC_MATRIX
#define CLASS_SRC_MATRIX

#include "matrix.hpp"
#include <cmath>
#include <iostream>

/******************************************************************************
 * __Matrix__
 * 
 * @details 	The default [Matrix] constructor.
 ******************************************************************************/
template<class T>
Matrix<T>::Matrix(const int &N)
: Matrix(N, N)
{
	//
}

/******************************************************************************
 * __Matrix__
 * 
 * @details 	
 ******************************************************************************/
template<class T>
Matrix<T>::Matrix(const int &a_noColumns, const int &a_noRows)
{
	this->noColumns = a_noColumns;
	this->noRows = a_noRows;
	items.reserve(noRows * noColumns);
}

/******************************************************************************
 * __Matrix__
 * 
 * @details 	
 ******************************************************************************/
template<class T>
Matrix<T>::Matrix(const int &a_noColumns, const int &a_noRows, const T &a_initial)
: Matrix(a_noColumns, a_noRows)
{
	for (int i=0; i<noColumns; ++i)
		for (int j=0; j<noRows; ++j)
			items[get_index(i, j)] = a_initial;
}

/******************************************************************************
 * __Matrix__
 * 
 * @details 	
 ******************************************************************************/
template<class T>
Matrix<T>::Matrix(const Matrix<T> &a_matrix)
: Matrix(a_matrix.get_noColumns(), a_matrix.get_noRows())
{
	for (int i=0; i<noColumns; ++i)
		for (int j=0; j<noRows; ++j)
			items[get_index(i, j)] = a_matrix(i, j);
}

/******************************************************************************
 * __get_noRows__
 * 
 * @details 	
 ******************************************************************************/
template<class T>
int Matrix<T>::get_noRows() const
{
	return this->noRows;
}

/******************************************************************************
 * __get_noColumns__
 * 
 * @details 	
 ******************************************************************************/
template<class T>
int Matrix<T>::get_noColumns() const
{
	return this->noColumns;
}

/******************************************************************************
 * __get_diagonal__
 * 
 * @details 	
 ******************************************************************************/
template<class T>
std::vector<T> Matrix<T>::get_diagonal() const
{
	int diagonalLength = noColumns<noRows?noColumns:noRows;
	std::vector<T> diagonal(diagonalLength, 0);

	for (int i=0; i<diagonalLength; ++i)
		diagonal[i] = item(i, i);

	return diagonal;
}

/******************************************************************************
 * __get_index__
 * 
 * @details 	
 ******************************************************************************/
template<class T>
int Matrix<T>::get_index(const int &a_x, const int &a_y) const
{
	if (a_x >= noColumns || a_y >= noRows)
	{
		std::cerr << "Error: Requested indices exceed matrix dimensions." << std::endl;
		return 0;
	}

	return a_x + a_y*noColumns;
}

/******************************************************************************
 * __item__
 * 
 * @details 	
 ******************************************************************************/
template<class T>
T& Matrix<T>::item(const int &a_x, const int &a_y)
{
	return items[get_index(a_x, a_y)];
}

/******************************************************************************
 * __item__
 * 
 * @details 	
 ******************************************************************************/
template<class T>
const T& Matrix<T>::item(const int &a_x, const int &a_y) const
{
	return items[get_index(a_x, a_y)];
}

/******************************************************************************
 * __operator()__
 * 
 * @details 	
 ******************************************************************************/
template<class T>
T& Matrix<T>::operator()(const int &a_x, const int &a_y)
{
	return item(a_x, a_y);
}

/******************************************************************************
 * __operator()__
 * 
 * @details 	
 ******************************************************************************/
template<class T>
const T& Matrix<T>::operator()(const int &a_x, const int &a_y) const
{
	return item(a_x, a_y);
}

/******************************************************************************
 * __operator=__
 * 
 * @details 	
 ******************************************************************************/
template<class T>
Matrix<T>& Matrix<T>::operator=(const Matrix<T> &a_RHS)
{
	if (&a_RHS == this)
		return *this;

	this->noRows = a_RHS.get_noRows();
	this->noColumns = a_RHS.get_noColumns();

	items.reserve(noRows * noColumns);

	for (int i=0; i<noColumns; ++i)
		for (int j=0; j<noRows; ++j)
			item(i, j) = a_RHS(i, j);

	return *this;
}

/******************************************************************************
 * __operator+__
 * 
 * @details 	
 ******************************************************************************/
template<class T>
Matrix<T> Matrix<T>::operator+(const Matrix<T> &a_RHS)
{
	// Creates new matrix and calculates elements appropriately.
	Matrix<T> tempMatrix((*this));
	
	tempMatrix += a_RHS;

	return tempMatrix;
}

/******************************************************************************
 * __operator+=__
 * 
 * @details 	
 ******************************************************************************/
template<class T>
Matrix<T>& Matrix<T>::operator+=(const Matrix<T> &a_RHS)
{
	// Dimensions must be the same.
	if (noRows != a_RHS.get_noRows() || noColumns != a_RHS.get_noColumns())
	{
		std::cerr << "Matrix dimensions do not match.";
		return *this;
	}

	// Creates new matrix and calculates elements appropriately.
	for (int i=0; i<noColumns; ++i)
		for (int j=0; j<noRows; ++j)
			item(i, j) += a_RHS(i, j); // I think it's a problem with this LHS -- I don't think you can write to it...

	return *this;
}

/******************************************************************************
 * __operator-__
 * 
 * @details 	
 ******************************************************************************/
template<class T>
Matrix<T> Matrix<T>::operator-(const Matrix<T> &a_RHS)
{
	// Creates new matrix and calculates elements appropriately.
	Matrix<T> tempMatrix((*this));
	
	tempMatrix -= a_RHS;

	return tempMatrix;
}

/******************************************************************************
 * __operator-=__
 * 
 * @details 	
 ******************************************************************************/
template<class T>
Matrix<T>& Matrix<T>::operator-=(const Matrix<T> &a_RHS)
{
	// Dimensions must be the same.
	if (noRows != a_RHS.get_noRows() || noColumns != a_RHS.get_noColumns())
	{
		std::cerr << "Matrix dimensions do not match.";
		return *this;
	}

	// Creates new matrix and calculates elements appropriately.
	for (int i=0; i<noColumns; ++i)
		for (int j=0; j<noRows; ++j)
			item(i, j) -= a_RHS(i, j); // I think it's a problem with this LHS -- I don't think you can write to it...

	return *this;
}

/******************************************************************************
 * __operator*__
 * 
 * @details 	
 ******************************************************************************/
template<class T>
Matrix<T> Matrix<T>::operator*(const Matrix<T> &a_RHS)
{
		// Matching dimensions.
	if (noRows != a_RHS.get_noColumns())
	{
		std::cerr << "Matrix dimensions do not match (cannot multiply "
			<< noColumns
			<< "x"
			<< noRows
			<< " by "
			<< a_RHS.get_noColumns()
			<< "x"
			<< a_RHS.get_noRows()
			<< ").";

		return *this;
	}

	int newRows = a_RHS.get_noRows();
	int newColumns = noColumns;

	Matrix<T> tempMatrix(newColumns, newRows, 0);

	// Creates new matrix and calculates elements appropriately.
	for (int i=0; i<newColumns; ++i)
		for (int j=0; j<newRows; ++j)
			for (int k=0; k<noRows; ++k)
				tempMatrix(i, j) += item(i, k) * a_RHS(k, j);

	return tempMatrix;
}

/******************************************************************************
 * __operator*__
 * 
 * @details 	
 ******************************************************************************/
template<class T>
Matrix<T> Matrix<T>::operator*(const T &a_RHS)
{
	Matrix<T> tempMatrix(*this);

	tempMatrix *= a_RHS;

	return tempMatrix;
}

/******************************************************************************
 * __operator*__
 * 
 * @details 	
 ******************************************************************************/
template<class T>
std::vector<T> Matrix<T>::operator*(const std::vector<T> &a_RHS)
{
	if (noColumns != a_RHS.size())
	{
		std::cerr << "Error: Matrix-vector dimensions do not match." << std::endl;
		return a_RHS;
	}

	std::vector<T> tempVector(noColumns, 0);
	for (int i=0; i<noColumns; ++i)
		for (int j=0; j<noRows; ++j)
			tempVector[i] += item(i, j) * a_RHS[j];

	return tempVector;
}

/******************************************************************************
 * __operator*=__
 * 
 * @details 	
 ******************************************************************************/
template<class T>
Matrix<T>& Matrix<T>::operator*=(const Matrix<T> &a_RHS)
{
	// Matching dimensions.
	if (noRows != a_RHS.get_noColumns() || noRows != a_RHS.get_noRows())
	{
		std::cerr << "Error: Matrix dimensions do not match." << std::endl;
		return *this;
	}

	Matrix<T> tempMatrix(noColumns, noRows, 0);

	// Creates new matrix and calculates elements appropriately.
	for (int i=0; i<noColumns; ++i)
		for (int j=0; j<noRows; ++j)
			for (int k=0; k<noRows; ++k)
				tempMatrix(i, j) += item(i, k) * a_RHS(k, j);

	*this = tempMatrix;

	return *this;
}

/******************************************************************************
 * __operator*=__
 * 
 * @details 	
 ******************************************************************************/
template<class T>
Matrix<T>& Matrix<T>::operator*=(const T &a_RHS)
{
	// Creates new matrix and calculates elements appropriately.
	for (int i=0; i<noColumns; ++i)
		for (int j=0; j<noRows; ++j)
			item(i, j) *= a_RHS;

	return *this;
}

/******************************************************************************
 * __operator/__
 * 
 * @details 	
 ******************************************************************************/
template<class T>
Matrix<T> Matrix<T>::operator/(const T &a_RHS)
{
	Matrix<T> tempMatrix(*this);

	tempMatrix /= a_RHS;

	return tempMatrix;
}

/******************************************************************************
 * __operator/=__
 * 
 * @details 	
 ******************************************************************************/
template<class T>
Matrix<T>& Matrix<T>::operator/=(const T &a_RHS)
{
	// Creates new matrix and calculates elements appropriately.
	for (int i=0; i<noColumns; ++i)
		for (int j=0; j<noRows; ++j)
			item(i, j) /= a_RHS;

	return *this;
}

#endif