#ifndef MESH_HPP
#define MESH_HPP

#include "element.hpp"

class Mesh
{
	private:
		int noElements;
		int noNodes;
		int dimProblem;

	public:
		Elements* elements;
		Mesh(const int &a_noElements);
		~Mesh();

		int get_dimProblem();
		int get_noNodes();
		// Also store 'faces', with many 'face's.
};

#endif