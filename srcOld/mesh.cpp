/******************************************************************************
 * @details This is a file containing declarations of [mesh].
 * 
 * @author     Adam Matthew Blakey
 * @date       2019/12/03
 ******************************************************************************/
#include "element.hpp"
#include "mesh.hpp"

/******************************************************************************
 * __Mesh__
 * 
 * @details 	The Mesh constructor, taking 1 argument for the number of elements.
 * 
 * @param[in] a_noElements 		The number of elements in this mesh. 	
 ******************************************************************************/
Mesh::Mesh(const int &a_noElements)
{
	this->noElements = a_noElements;
	this->noNodes    = a_noElements+1;
	this->dimProblem = a_noElements+1;
	this->elements = new Elements(a_noElements);
}

/******************************************************************************
 * __~Mesh__
 * 
 * @details 	Destroys memory for an instance of Mesh.
 ******************************************************************************/
Mesh::~Mesh()
{
	delete this->elements;
}

/******************************************************************************
 * __get_dimProblem__
 * 
 * @details 	Returns the value of the private variable 'dimProblem'.
 ******************************************************************************/
int Mesh::get_dimProblem()
{
	return this->dimProblem;
}

/******************************************************************************
 * __get_noNodes__
 * 
 * @details 	Returns the value of the private variable 'noNodes'.
 ******************************************************************************/
int Mesh::get_noNodes()
{
	return this->noNodes;
}