#include "common.hpp"
#include "element.hpp"
#include "matrix.hpp"
#include "mesh.hpp"
#include "refinement.hpp"
#include "solution.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>

#include <functional>

double zero(double x)
{
	return 0;
}

double one(double x)
{
	return 1;
}

double m_one(double x)
{
	return -1;
}

double pi2sin(double x)
{
	return pow(M_PI, 2) * sin(M_PI * x);
}

double sinpi(double x)
{
	return sin(M_PI * x);
}

double exact(double x)
{
	return -cosh(pow(10, double(3)/2)*x)/cosh(pow(10, double(3)/2)) + 1;
}

double exact_(double x)
{
	return -sinh(pow(10, double(3)/2)*x)/cosh(pow(10, double(3)/2)) + 1;
}

double expx2(double x)
{
	return exp(-pow(x, 2));
}

void lrefinement();

double adam(double x)
{
	double a = 1e-3;

	return -exp(x/sqrt(a))/(exp(double(1)/sqrt(a)) + 1) - (exp(-x/sqrt(a)) * exp(double(1)/sqrt(a)))/(exp(double(1)/sqrt(a)) + 1) + 1;
}

int main()
{
	// Sets up problem.
	Mesh*     myMesh     = new Mesh(4);
	//Solution* mySolution = new Solution(myMesh, one, 1e-3, one);

	Solution* mySolution = new Solution(myMesh, one, 1e-3, one);

	//Solution* mySolution = new Solution(myMesh, pi2sin, 1, zero);
	Mesh*     myNewMesh;
	Solution* myNewSolution;
	double errorIndicator, errorIndicatorPrev = 0;
	double tolerance = 1e-5;
	int maxIterations = 10;
	int iteration;

	refinement::refinement(myMesh, &myNewMesh, mySolution, &myNewSolution, tolerance, maxIterations, true, true, true);
	myNewSolution->Solve(1e-15);
	//myNewSolution->outputToFile(sinpi);
	//myNewSolution->outputToFile(exact);
	myNewSolution->outputToFile(adam);




	std::ofstream outputFile;
	outputFile.open("refinement.dat");
	assert(outputFile.is_open());

	int n = myNewMesh->get_noElements();

	for (int i=0; i<n; ++i)
	{
		Element* currentElement = (*(myNewMesh->elements))[i];

		outputFile
			<< std::setw(26) << std::setprecision(16) << std::scientific << currentElement->get_nodeCoordinates()[0]
			<< std::setw(26) << std::setprecision(16) << std::scientific << currentElement->get_nodeCoordinates()[1]
			<< std::setw(26) << std::setprecision(16) << std::scientific << currentElement->get_polynomialDegree()
		<< std::endl;
	}

	outputFile.close();








	delete mySolution;
	delete myMesh;




	//lrefinement();

	return 0;
}

void lrefinement()
{
	// Sets up problem.
	Mesh*     myMesh     = new Mesh(10);
	Solution* mySolution = new Solution(myMesh, one, 1e-3, one);
	double errorIndicator, errorIndicatorPrev = 0;
	double tolerance = 1e-3;
	int maxIterations = 50;
	int iteration;

	// Temporary variables.
	Mesh* currentMesh;
	Mesh* newMesh = myMesh;
	Solution* currentSolution;
	Solution* newSolution = mySolution;

	double eNorm, eNormPrev = 0;

	// Loops whilst we've still got 
	
	for (iteration=0; iteration<maxIterations; ++iteration)
	{
		// Passes pointers from each iteration.
		currentSolution = newSolution;
		currentMesh = newMesh;

		// Solve.
		currentSolution->Solve(1e-3);
		currentSolution->outputToFile(exact);

		// Calculates new error indicator.
		errorIndicator = currentSolution->compute_globalErrorIndicator();
		if (errorIndicator <= tolerance)
			break;

		// Error indicators calculation.
		std::vector<double> errorIndicators = currentSolution->compute_errorIndicators();

		// [True error]
		//Maybe ask to stop?
		
		eNorm = currentSolution->compute_energyNormDifference2(exact, exact_);

		std::cout << "#Elements       : " << currentMesh->get_noElements() << std::endl;
		std::cout << "Error indicator : " << errorIndicator << std::endl;
		std::cout << "Max indicator   : " << *std::max_element(errorIndicators.begin(), errorIndicators.end()) << std::endl;
		std::cout << "Energy norm     : " << eNorm << std::endl;
		std::cout << "Energy ratio    : " << eNormPrev/eNorm << std::endl;
		std::cout << "Indicator ratio : " << errorIndicatorPrev/errorIndicator << std::endl;
		std::cout << std::endl;
		system("PAUSE");

		eNormPrev = eNorm;
		errorIndicatorPrev = errorIndicator;
		
		// Refine and create new mesh and solution.
		refinement::refine_p(currentMesh, &newMesh, currentSolution, &newSolution, errorIndicators);

		delete currentMesh;
		delete currentSolution;
		currentMesh = newMesh;
		currentSolution = newSolution;
	}

	currentSolution->outputToFile(exact);

	std::cout << "Completed with:" << std::endl;
	std::cout << "  #Elements      : " << currentMesh->get_noElements() << std::endl;
	std::cout << "  Error indicator: " << currentSolution->compute_globalErrorIndicator() << std::endl;
	std::cout << "  #Iterations    : " << iteration << std::endl;

	delete currentSolution;
	delete currentMesh;
}