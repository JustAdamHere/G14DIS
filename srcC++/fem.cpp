/*=============================================================================
 * FEM.CPP
 *
 * This is a file containing functions regarding the FEM related functions.
 * 
 * @author     Adam Matthew Blakey
 * @date       2019/11/11
  =============================================================================*/

#include "common.cpp"
#include "linearSystems.cpp"
#include "quadrature.cpp"
#include <cmath>
#include <functional>

// TODO - ONLY FOR DEBUGGING!!!
#include <iostream>

using namespace std;

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
  		  ___  ___ ___ _      _   ___    _ _____ ___ ___  _  _ ___
		 |   \| __/ __| |    /_\ | _ \  /_\_   _|_ _/ _ \| \| / __|
		 | |) | _| (__| |__ / _ \|   / / _ \| |  | | (_) | .` \__ \
		 |___/|___\___|____/_/ \_\_|_\/_/ \_\_| |___\___/|_|\_|___/

 *$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/

/******************************************************************************
 * basis0_
 * 
 * @details    Returns the derivative of \phi_0 for a global h.
 *
 * @param[in] h 	Global h.
 ******************************************************************************/

double basis0_(const double h);

/******************************************************************************
 * basis1_
 * 
 * @details    Returns the derivative of \phi_1 for a global h.
 *
 * @param[in] h 	Global h.
 ******************************************************************************/

double basis1_(const double h);

/******************************************************************************
 * basis0
 * 
 * @details    Returns the value of \phi_0 at x.
 *
 * @param[in] x 	The point to evaluate at.
 ******************************************************************************/

double basis0(const double x);

/******************************************************************************
 * basis1
 * 
 * @details    Returns the value of \phi_1 at x.
 *
 * @param[in] x 	The point to evaluate at.
 ******************************************************************************/

double basis1(const double x);

/******************************************************************************
 * l
 * 
 * @details    Returns the result of the linear form at a particular element.
 *
 * @param[in] j 				Basis function number.
 * @param[in] node1Index		Index of node1.
 * @param[in] node1 			The left-hand node of the element.
 * @param[in] node2 			The right-hand node of the element.
 * @param[in] p 				The function p.
 * @param[in] q 				The function, q.
 ******************************************************************************/

double l(int j, int node1Index, double node1, double node2, function<double(double)> f);

/******************************************************************************
 * a
 * 
 * @details    Returns the result of the bilinear form at a particular element.
 *
 * @param[in] i 				First basis function number.
 * @param[in] j 				Second basis function number.
 * @param[in] node1Index 		Index of node1.
 * @param[in] node1 			The left-hand node of the element.
 * @param[in] node2 			The right-hand node of the element.
 * @param[in] p 				The function p.
 * @param[in] q 				The function, q.
 ******************************************************************************/

double a(int i, int j, int node1Index, double node1, double node2, function<double(double)> p, function<double(double)> q);

/******************************************************************************
 * linearFEM
 * 
 * @details    Returns the approximate solution, u, to the problem
 * 				-(pu')' + qu = f for x in (0, 1) with u(0) = A and u(1) = B
 *				with linear basis functions.
 *
 * @param[in] n 				Number of elements.
 * @param[in] nodes[] 			The mesh nodes on the interval [0, 1].
 * @param[in] p 				Pointer to the function p.
 * @param[in] q 				Pointer to the function q.
 * @param[in] f 				Pointer to the function f.
 * @param[in] A 				Value of u(0).
 * @param[in] B 				Value of u(1).
 * @param[out] solution			Returns the approximation of the solution.
 ******************************************************************************/

void linearFEM(const int n, const double nodes[], function<double(double)> p, function<double(double)> q, function<double(double)> f, const double A, const double B, double solution[]);

/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
			  ___  ___ ___ ___ _  _ ___ _____ ___ ___  _  _ ___ 
			 |   \| __| __|_ _| \| |_ _|_   _|_ _/ _ \| \| / __|
			 | |) | _|| _| | || .` || |  | |  | | (_) | .` \__ \
			 |___/|___|_| |___|_|\_|___| |_| |___\___/|_|\_|___/
			                                                  
 *$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/

/******************************************************************************
 * basis0_
 ******************************************************************************/

double basis0_(const double h)
{
	return -double(1)/h;
}

/******************************************************************************
 * basis1_
 ******************************************************************************/

double basis1_(const double h)
{
	return double(1)/h;
}

/******************************************************************************
 * basis0
 ******************************************************************************/

double basis0(const double x)
{
	if (0 <= x && x <= 1)
		return 1-x;
	else
		return 0;
}

/******************************************************************************
 * basis1
 ******************************************************************************/

double basis1(const double x)
{
	if (0 <= x && x <= 1)
		return x;
	else
		return 0;
}

/******************************************************************************
 * l
 ******************************************************************************/

double l(int j, int node1Index, double node1, double node2, function<double(double)> f)
{
	int n = 101; ///< Number of sub-elements to split our element into.
	double* fValues = new double[n];

	double h  = node2 - node1; ///< Global h.
	double h_ = (node2-node1)/(n-1); ///< Local h for creating our sub-elements.

	double node;
	for (int a=0; a<n; ++a)
	{
		node = node1 + h_*a;

		if (j == node1Index)
			fValues[a] = f(node)*basis1((node - node1)/h);
		else
			fValues[a] = f(node)*basis0((node - node1)/h);
	}

	return basisTrapeziumRule(n, fValues)*h; // Does integration on reference element.

	delete[] fValues;
}

/******************************************************************************
 * a
 ******************************************************************************/

double a(int i, int j, int node1Index, double node1, double node2, function<double(double)> p, function<double(double)> q)
{
	int n = 101; ///< Number of sub-elements to split our element into.
	double* nodes = new double[n]; 
	double* fValues = new double[n];

	double h  = node2 - node1; ///< Global h.
	double h_ = (node2-node1)/(n-1); ///< Local h for creating our sub-elements.

	// Scale for h?????????
	for (int a=0; a<n; ++a)
	{
		nodes[a] = node1 + h_*a;
		if (i==j)
		{
			if (i==node1Index)
				fValues[a] = p(nodes[a])*basis1_(h)*basis1_(h) + q(nodes[a])*basis1((nodes[a] - node1)/h)*basis0((nodes[a] - node1)/h);
			else
				fValues[a] = p(nodes[a])*basis0_(h)*basis0_(h) + q(nodes[a])*basis1((nodes[a] - node1)/h)*basis0((nodes[a] - node1)/h);
		}
		else
			fValues[a] = p(nodes[a])*basis1_(h)*basis0_(h) + q(nodes[a])*basis1((nodes[a] - node1)/h)*basis0((nodes[a] - node1)/h);
	}

	return basisTrapeziumRule(n, fValues)*h; // Does integration on reference element.

	delete[] fValues;
	delete[] nodes;
}

/******************************************************************************
 * linearFEM
 ******************************************************************************/

void linearFEM(const int n, const double nodes[], function<double(double)> p, function<double(double)> q, function<double(double)> f, const double A, const double B, double solution[])
{
	// ************************************************************************
	// Sets up structure of stiffness matrix and load vector, and sets to zero.
	// ************************************************************************
	double* A1 = new double[n-1]; ///< Stiffness matrix A's lower diagonal.
	double* A2 = new double[n]; ///< Stiffness matrix A's diagonal.
	double* A3 = new double[n-1]; ///< Stiffness matrix A's upper diagonal.
	double* F  = new double[n]; ///< Load vector F.

	setToZero(n-1, A1);
	setToZero(n, A2);
	setToZero(n-1, A3);
	setToZero(n, F);

	// ************************
	// Loops over each element.
	// ************************
	for (int meshCounter=0; meshCounter<=n-2; ++meshCounter)
	{
		double meshLeft = nodes[meshCounter];
		double meshRight = nodes[meshCounter+1];

		// *******************************************************
		// Loops over both adjacent nodes twice (nested loop).
		// *******************************************************
		// Here's an example of the numbering used...
		// -------------------------------------------------------
		// 
		//			            N=5 EXAMPLE
		//			              
		//				          ELEMENTS
		//				 |--0--|--1--|--2--|--3--|
		//				 
		//				    ^     ^     ^     ^ 
		//				    |     |     |     |
		//				   E_0   E_1   E_2   E_3
		//				p_0   p_1   p_2   p_3   p_4
		//				 |     |     |     |     |
		//				 v     v     v     v     v
		//				
		//				 \     ^     ^     ^     /
		//				  \   / \   / \   / \   /
		//				   \ /   \ /   \ /   \ /
		//				    X     X     X     X
		//				   / \   / \   / \   / \
		//				  /   \ /   \ /   \ /   \
		//				 0-----1-----2-----3-----4
		//			             NODES
		//             
		// where p are the basis functions and E are the elements.
		// 
		// So element 2 is neighboured by nodes 2 and 3.
		// *******************************************************
		for (int j=meshCounter; j<=meshCounter+1; ++j)
		{
			F[j] += l(j, meshCounter, meshLeft, meshRight, f); 

			for (int i=meshCounter; i<=meshCounter+1; ++i)
			{
				if (j<i)
					A1[j] += a(i, j, meshCounter, meshLeft, meshRight, p, q);
				else if (j==i)
					A2[i] += a(i, j, meshCounter, meshLeft, meshRight, p, q);
				else
					A3[i] += a(i, j, meshCounter, meshLeft, meshRight, p, q);
			}		
		}
	}

	

	// **************************************************************************
	// We know the solution at the end points, so reduce the problem accordingly.
	// **************************************************************************
	double* F_ = new double[n]; // The load vector contributions from boundary nodes.
	double* u0 = new double[n]; // Holds information regarding the boundary conditions.
	
	// Zeros 0th row of stiffness matrix.
	A2[0] = 0;
	A3[0] = 0;
	F[0] = 0;

	// Zeros (n-1)th row of stiffness matrix.
	A1[n-2] = 0;
	A2[n-1] = 0;
	F[n-1] = 0;

	// The solution at boundary points; zero elsewhere.
	setToZero(n, u0);
	u0[0]   = A;
	u0[n-1] = B;
	
	// Calculates contributions from boundary nodes and adds it.
	tridiagonalVectorMultiplication(n, A1, A2, A3, u0, F_); 
	for (int i=0; i<n; ++i)
	{
		F[i] -= F_[i];
	}

	delete[] u0;

	// Zeros 0th column and makes diagonal entry 1.
	A1[0] = 0;
	A2[0] = 1;

	// Zeros (n-1)th column and makes diagonal entry 1.
	A2[n-1] = 1;
	A3[n-2] = 0;





	// TODO - FOR DEBUGGING!
	/*for (int i=0; i<n-1; ++i)
	{
		cout << "A1_" << i << ": " << A1[i] << endl;
	}
	cout << endl;
	for (int i=0; i<n; ++i)
	{
		cout << "A2_" << i << ": " << A2[i] << endl;
	}
	cout << endl;
	for (int i=0; i<n-1; ++i)
	{
		cout << "A3_" << i << ": " << A3[i] << endl;
	}
	cout << endl;
	for (int i=0; i<n; ++i)
	{
		cout << "F_" << i << ": " << F[i] << endl;
	}
	cout << endl;*/





	// ***************************************
	// Inverts the reduced tridiagonal matrix.
	// ***************************************
	thomasInvert(n, A1, A2, A3, F, solution);

	// ****************************
	// Imposes boundary conditions.
	// ****************************
	solution[0]   = A;
	solution[n-1] = B;

	// **************************************************
	// Frees memory for stiffness matrix and load vector.
	// **************************************************
	delete[] F;
	delete[] A3;
	delete[] A2;
	delete[] A1;
}