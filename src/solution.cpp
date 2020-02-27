/******************************************************************************
 * @details This is a file containing definitions of [Solution].
 * 
 * @author     Adam Matthew Blakey
 * @date       2019/12/07
 ******************************************************************************/
#include "common.hpp"
#include "element.hpp"
#include "linearSystems.hpp"
#include "matrix.hpp"
#include "matrix_full.hpp"
#include "mesh.hpp"
#include "quadrature.hpp"
#include "solution.hpp"

#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

/******************************************************************************
 * __Solution__
 * 
 * @details 	The Mesh constructor, taking 1 argument for the mesh.
 * 
 * @param[in] a_mesh 		The mesh the solution is defined on.
 ******************************************************************************/
Solution::Solution(Mesh* const &a_mesh, f_double const &a_f, const double &a_epsilon, f_double const &a_c) // Something bad happens when you call this directly...
{
	this->noElements		= a_mesh->get_noElements();
	this->solution 			.resize(a_mesh->get_noNodes());
	this->boundaryConditions.resize(2);
	this->mesh 				= a_mesh;
	this->f       = a_f; 
	this->epsilon = a_epsilon;
	this->c       = a_c;
}

Solution::Solution(Mesh* const &a_mesh, f_double const &a_f, const double &a_epsilon, f_double const &a_c, f_double const &a_exact)
: Solution(a_mesh, a_f, a_epsilon, a_c)
{
	this->exact_u = a_exact;
}

Solution::Solution(Mesh* const &a_mesh, f_double const &a_f, const double &a_epsilon, f_double const &a_c, f_double const &a_exact, f_double const &a_exact_1)
: Solution(a_mesh, a_f, a_epsilon, a_c)
{
	this->exact_u   = a_exact;
	this->exact_u_1 = a_exact_1;
}

/******************************************************************************
 * __~Solution__
 ******************************************************************************/
Solution::~Solution()
{
	//
}

/******************************************************************************
 * __Solve__
 * 
 * @details 	Uses the stored data to calculate and populate the value in
 * 					local variable 'solution'.
 ******************************************************************************/
void Solution::Solve(const double &a_cgTolerance)
{
	double A = 0;
	double B = 0;

	int n = this->noElements + 1; // Number of nodes.

	Elements* elements = this->mesh->elements;

	Matrix_full<double> stiffnessMatrix(n, n, 0);
	std::vector<double> loadVector(n, 0);

	for (int elementCounter=0; elementCounter<this->noElements; ++elementCounter)
	{
		Element* currentElement = (*(this->mesh->elements))[elementCounter];

		double elementLeft  = currentElement->get_nodeCoordinates()[0];
		double elementRight = currentElement->get_nodeCoordinates()[1];

		std::vector<int> elementDoFs = elements->get_elementDoFs(elementCounter);
		for (int a=0; a<elementDoFs.size(); ++a)
		{
			int j = elementDoFs[a];
			loadVector[j] += this->l(currentElement, j, elementCounter);

			for (int b=0; b<elementDoFs.size(); ++b)
			{
				int i = elementDoFs[b];
				double value = stiffnessMatrix(i, j);
				stiffnessMatrix.set(i, j, value + this->a(currentElement, i, j, elementCounter));
			}
		}
	}

	std::vector<double> F_(n);
	std::vector<double> u0(n, 0);
	
	stiffnessMatrix.set(0, 0, 0);
	stiffnessMatrix.set(0, 1, 0);
	loadVector[0] = 0;

	stiffnessMatrix.set(n-1, n-1, 0);
	stiffnessMatrix.set(n-1, n-2, 0);
	loadVector[n-1] = 0;

	u0[0]   = A;
	u0[n-1] = B;
	
	F_ = stiffnessMatrix*u0;
	for (int i=0; i<n; ++i)
		loadVector[i] -= F_[i];

	stiffnessMatrix.set(0, 1, 0);
	stiffnessMatrix.set(0, 0, 1);

	stiffnessMatrix.set(n-1, n-1, 1);
	stiffnessMatrix.set(n-1, n-2, 0);

	this->solution = linearSystems::conjugateGradient(stiffnessMatrix, loadVector, a_cgTolerance);

	this->solution[0]   = A;
	this->solution[n-1] = B;
}

/******************************************************************************
 * __Solve__
 * 
 * @details 	Uses the stored data to calculate and populate the value in
 * 					local variable 'solution'.
 ******************************************************************************/
void Solution::Solve(const double &a_cgTolerance, const double &a_refTolerance, const int &a_maxNoElements)
{
	int noElements = this->mesh->get_noElements();
	assert(noElements <= a_maxNoElements);

	// Temporarily use another mesh.
	Mesh* oldMesh = this->mesh; // There's probably a better way.
	this->Solve(a_cgTolerance);

	// Variables for the algorithm.
	bool checksRequired = true;
	std::vector<bool> localChecks(noElements, true);

	while (checksRequired && noElements <= a_maxNoElements)
	{
		for (int i=0; i<this->mesh->get_noElements(); ++i)
		{

		}
	}

	// Put the old mesh back.
	this->mesh = oldMesh;
}

double Solution::l(Element* currentElement, const int &j, const int &node1Index)
{
	double J = currentElement->get_Jacobian();
	double integral = 0;
	
	std::vector<double> coordinates;
	std::vector<double> weights;
	currentElement->get_elementQuadrature(coordinates, weights);

	for (int k=0; k<coordinates.size(); ++k)
	{
		double b_value;
		if (j == node1Index)
			b_value = currentElement->basisFunctions(0)(coordinates[k]);
		else
			b_value = currentElement->basisFunctions(1)(coordinates[k]);

		double f_value = this->f(currentElement->mapLocalToGlobal(coordinates[k]));
		integral += b_value*f_value*weights[k]*J;
	}

	return integral;
}

double Solution::a(Element* currentElement, const int &i, const int &j, const int &node1Index)
{
	double J = currentElement->get_Jacobian();
	double integral = 0;
	
	std::vector<double> coordinates;
	std::vector<double> weights;
	currentElement->get_elementQuadrature(coordinates, weights);

	for (int k=0; k<coordinates.size(); ++k)
	{
		double b_value;
		if (i==j)
		{
			if (i==node1Index)
			{
				b_value = currentElement->basisFunctions_(0)(coordinates[k])
						* currentElement->basisFunctions_(0)(coordinates[k]);
			}
			else
			{
				b_value = currentElement->basisFunctions_(1)(coordinates[k])
						* currentElement->basisFunctions_(1)(coordinates[k]);
			}
		}
		else
		{
			b_value = currentElement->basisFunctions_(0)(coordinates[k])
					* currentElement->basisFunctions_(1)(coordinates[k]);
		}

		integral += this->epsilon*b_value*weights[k]/J;
	}

	for (int k=0; k<coordinates.size(); ++k)
	{
		double b_value;
		if (i==j)
		{
			if (i==node1Index)
			{
				b_value = currentElement->basisFunctions(1)(coordinates[k])
						* currentElement->basisFunctions(1)(coordinates[k]);
			}
			else
			{
				b_value = currentElement->basisFunctions(0)(coordinates[k])
						* currentElement->basisFunctions(0)(coordinates[k]);
			}
		}
		else
		{
			b_value = currentElement->basisFunctions(0)(coordinates[k])
					* currentElement->basisFunctions(1)(coordinates[k]);
		}

		double c_value = this->c(currentElement->mapLocalToGlobal(coordinates[k]));

		integral += c_value*b_value*weights[k]*J;
	}

	return integral;
}

f_double Solution::get_solutionInterpolant() const
{
	return [=](double x) -> double
	{
		int n = this->noElements;

		int i;
		bool foundRange = false;
		double node1, node2;

		for (i=0; i<n && !foundRange;)
		{
			node1 = (*(this->mesh->elements))[i]->get_nodeCoordinates()[0];
			node2 = (*(this->mesh->elements))[i]->get_nodeCoordinates()[1];

			//std::cout << "(" << node1 << ", " << node2 << "): " << x << std::endl;

			if (node1<=x && x<=node2)
			{
				foundRange = true;
			}
			else
			{
				++i;
			}
		}

		//std::cout << "(" << node1 << ", " << node2 << "): " << x << std::endl;
		//std::cout << "i: " << i << std::endl;
		//std::cout << "x^: " << 2*(x-node1)/(node2 - node1) - 1 << std::endl;
		//std::cout << "(" << this->solution[i] << ", " << this->solution[i+1] << ")" << std::endl;

		if (foundRange)
		{
			f_double f1 = common::constantMultiplyFunction(this->solution[i],   (*(this->mesh->elements))[i]->basisFunctions(0));
			f_double f2 = common::constantMultiplyFunction(this->solution[i+1], (*(this->mesh->elements))[i]->basisFunctions(1));
			/*f_double f1 = common::constantMultiplyFunction(1,   (*(this->mesh->elements))[i]->basisFunctions(0));
			f_double f2 = common::constantMultiplyFunction(0, (*(this->mesh->elements))[i]->basisFunctions(1));*/

			return common::addFunction(f1, f2)(2*(x-node1)/(node2-node1) - 1);
			//return common::addFunction(f1, f2)(x);
		}
		else
		{
			return 0;
		}
	};
}

f_double Solution::get_solutionInterpolant_() const
{
	return [=](double x) -> double
	{
		int n = this->noElements;

		int i;
		bool foundRange = false;
		double node1, node2;

		for (i=0; i<n && !foundRange;)
		{
			node1 = (*(this->mesh->elements))[i]->get_nodeCoordinates()[0];
			node2 = (*(this->mesh->elements))[i]->get_nodeCoordinates()[1];

			//std::cout << "(" << node1 << ", " << node2 << "): " << x << std::endl;

			if (node1<=x && x<=node2)
			{
				foundRange = true;
			}
			else
			{
				++i;
			}
		}

		//std::cout << "(" << node1 << ", " << node2 << "): " << x << std::endl;
		//std::cout << "i: " << i << std::endl;
		//std::cout << "x^: " << 2*(x-node1)/(node2 - node1) - 1 << std::endl;
		//std::cout << "(" << this->solution[i] << ", " << this->solution[i+1] << ")" << std::endl;

		if (foundRange)
		{
			f_double f1 = common::constantMultiplyFunction(this->solution[i],   (*(this->mesh->elements))[i]->basisFunctions_(0));
			f_double f2 = common::constantMultiplyFunction(this->solution[i+1], (*(this->mesh->elements))[i]->basisFunctions_(1));
			/*f_double f1 = common::constantMultiplyFunction(1,   (*(this->mesh->elements))[i]->basisFunctions(0));
			f_double f2 = common::constantMultiplyFunction(0, (*(this->mesh->elements))[i]->basisFunctions(1));*/

			return common::addFunction(f1, f2)(2*(x-node1)/(node2-node1) - 1);
			//return common::addFunction(f1, f2)(x);
		}
		else
		{
			return 0;
		}
	};
}

double Solution::get_L2Norm() const
{
	int n = this->mesh->get_noElements();

	double norm = 0;

	for (int i=0; i<n; ++i)
	{
		// Gets the current element.
		Element* currentElement = (*(this->mesh->elements))[i];

		// Retrieves quadrature information.
		std::vector<double> coordinates;
		std::vector<double> weights;
		currentElement->get_elementQuadrature(coordinates, weights);

		for (int j=0; j<coordinates.size(); ++j)
		{
			// Actual and approximate solution at coordinates.
			double uh = compute_uh(i, coordinates[j]);
			double u = compute_u(currentElement->mapLocalToGlobal(coordinates[j]));

			double Jacobian = currentElement->get_Jacobian();
			//Matrix_full JacobiMatrix = currentElement->get_Jacobi();

			//std::cout << "weight: " << weights[j] << std::endl;

			norm += pow(u - uh, 2)*weights[j]*Jacobian; // Add on H1 when you get to it...
		}
	}

	return sqrt(norm);
}

double Solution::get_H1Norm() const
{
	int n = this->mesh->get_noElements();

	double norm = 0;

	for (int i=0; i<n; ++i)
	{
		// Gets the current element.
		Element* currentElement = (*(this->mesh->elements))[i];

		// Retrieves quadrature information.
		std::vector<double> coordinates;
		std::vector<double> weights;
		currentElement->get_elementQuadrature(coordinates, weights);

		for (int j=0; j<coordinates.size(); ++j)
		{
			// Actual and approximate solution at coordinates.
			double uh   = compute_uh  (i, coordinates[j]);
			double uh_1 = compute_uh_1(i, coordinates[j]);
			double u    = compute_u   (currentElement->mapLocalToGlobal(coordinates[j]));
			double u_1  = compute_u_1 (currentElement->mapLocalToGlobal(coordinates[j]));

			double Jacobian = currentElement->get_Jacobian();
			//Matrix_full JacobiMatrixIT = currentElement->get_Jacobi()->get_InverseTranspose();

			norm += pow(u   - uh  , 2)*weights[j]*Jacobian
				 +  pow(u_1 - uh_1, 2)*weights[j]*Jacobian;
		}
	}

	return sqrt(norm);
}

double Solution::get_energyNorm() const
{
	int n = this->mesh->get_noElements();
	double sqrt_epsilon = sqrt(this->epsilon);

	double norm = 0;

	for (int i=0; i<n; ++i)
	{
		// Gets the current element.
		Element* currentElement = (*(this->mesh->elements))[i];

		// Retrieves quadrature information.
		std::vector<double> coordinates;
		std::vector<double> weights;
		currentElement->get_elementQuadrature(coordinates, weights);

		for (int j=0; j<coordinates.size(); ++j)
		{
			// Actual and approximate solution at coordinates.
			double uh   = compute_uh  (i, coordinates[j]);
			double uh_1 = compute_uh_1(i, coordinates[j]);
			double u    = compute_u   (currentElement->mapLocalToGlobal(coordinates[j]));
			double u_1  = compute_u_1 (currentElement->mapLocalToGlobal(coordinates[j]));

			double Jacobian = currentElement->get_Jacobian();

			norm += pow(sqrt_epsilon*(u_1 - uh_1), 2)*weights[j]*Jacobian
				 +  pow(sqrt(this->c(coordinates[j]))*(u - uh), 2)*weights[j]*Jacobian;
		}
	}

	return sqrt(norm);
}

double Solution::compute_uh(const int &a_i, const double &a_xi) const
{
	f_double f1 = common::constantMultiplyFunction(this->solution[a_i],   (*(this->mesh->elements))[a_i]->basisFunctions(0));
	f_double f2 = common::constantMultiplyFunction(this->solution[a_i+1], (*(this->mesh->elements))[a_i]->basisFunctions(1));

	return common::addFunction(f1, f2)(a_xi);
}

double Solution::compute_uh_1(const int &a_i, const double &a_xi) const
{
	double J = (*(this->mesh->elements))[a_i]->get_Jacobian(); // Needs to be inverse transpose of Jacobi in dimensions higher than 1.

	f_double f1 = common::constantMultiplyFunction(this->solution[a_i],   (*(this->mesh->elements))[a_i]->basisFunctions_(0));
	f_double f2 = common::constantMultiplyFunction(this->solution[a_i+1], (*(this->mesh->elements))[a_i]->basisFunctions_(1));

	return common::addFunction(f1, f2)(a_xi) / J;
}

double Solution::compute_u(const double &a_x) const
{
	return this->exact_u(a_x);
}

double Solution::compute_u_1(const double &a_x) const
{
	return this->exact_u_1(a_x);
}

void Solution::outputToFile(const std::string a_filename) const
{
	std::ofstream outputFile;
	outputFile.open(a_filename);
	assert(outputFile.is_open());

	int n = this->mesh->get_noElements();

	for (int i=0; i<n; ++i)
	{
		Element* currentElement = (*(this->mesh->elements))[i];

		outputFile
			<< std::setw(26) << std::setprecision(16) << std::scientific << currentElement->get_nodeCoordinates()[0]
			<< std::setw(26) << std::setprecision(16) << std::scientific << this->solution[i]
			<< std::setw(26) << std::setprecision(16) << std::scientific << this->compute_u(currentElement->get_nodeCoordinates()[0])
		<< std::endl;
	}

	Element* lastElement = (*(this->mesh->elements))[n-1];
	outputFile
		<< std::setw(26) << std::setprecision(16) << std::scientific << lastElement->get_nodeCoordinates()[1]
		<< std::setw(26) << std::setprecision(16) << std::scientific << this->solution[n]
		<< std::setw(26) << std::setprecision(16) << std::scientific << this->compute_u(lastElement->get_nodeCoordinates()[1])
	<< std::endl;

	outputFile.close();
}

double Solution::get_globalErrorIndicator() const
{
	double errorIndicator = 0;

	for (int i=0; i<this->noElements; ++i)
		errorIndicator += compute_errorIndicator(i);

	return sqrt(errorIndicator);
}

double Solution::compute_errorIndicator(const double &a_i) const
{
	// Gets element and its properties.
	Element* currentElement = (*(this->mesh->elements))[a_i];
	int P = currentElement->get_polynomialDegree();
	double leftNode  = currentElement->get_nodeCoordinates()[0];
	double rightNode = currentElement->get_nodeCoordinates()[1];
	double Jacobian  = currentElement->get_Jacobian();

	// Calculates L2 norm on element with weight and residual.
	double norm_2 = 0;
	std::vector<double> quadratureCoordinates;
	std::vector<double> quadratureWeights;
	currentElement->get_elementQuadrature(quadratureCoordinates, quadratureWeights);

	// Loops over quadrature coordinates and weights.
	for (int j=0; j<quadratureCoordinates.size(); ++j)
	{
		double uh = compute_uh(a_i, quadratureCoordinates[j]);
		double residual = compute_residual(uh, quadratureCoordinates[j]);

		double x = currentElement->mapLocalToGlobal(quadratureCoordinates[j]);
		double weight = (rightNode - x)*(x - leftNode);

		norm_2 += pow(sqrt(weight)*residual, 2)*quadratureWeights[j]*Jacobian;
	}
	
	return double(1)/(P*(P+1)*this->epsilon) * norm_2;
}

double Solution::compute_residual(const double &a_uh, const double &a_x) const
{
	double a_uh_2 = 0;

	return this->f(a_x) + this->epsilon*a_uh_2 - this->c(a_x)*a_uh;
}

std::vector<int> Solution::get_higherOrderDoFs() const
{
	std::vector<int> DoFStarts(noElements+1, noElements+2);

	for (int i=0; i<noElements; ++i)
	{
		Element* currentElement = (*(this->mesh->elements))[i];

		DoFStarts[i] += currentElement->get_polynomialDegree() - 1;
	}

	return DoFStarts;
}