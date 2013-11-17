/*
 * powerset.h
 *
 *  Created on: Nov 17, 2013
 *      Author: nick
 */

#ifndef POWERSET_H_
#define POWERSET_H_

#include <vector>
#include <iostream>

using namespace std;


ostream& operator <<(ostream& io, const vector<int>& vec);

class SubsetsGenerator
{

public:
	vector< vector<int> > generateSubsets(int N);
	vector< vector<int> > generateSubsets2(int N);
	vector< vector<int> > generateSubsets3(int N);
	void printSubsets(const vector<vector<int> >& subsets);
};

#endif /* POWERSET_H_ */
