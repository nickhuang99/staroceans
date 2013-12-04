/*
 * powerset.cpp
 *
 *  Created on: Nov 17, 2013
 *      Author: nick
 */
#include <iostream>
#include <cstdio>
#include <string>
#include <stdint.h>
#include <bitset>
#include <ctime>
#include <boost/dynamic_bitset.hpp>

#include "powerset.h"

using namespace std;



// assume N is smaller than 64
vector< vector<int> > SubsetsGenerator::generateSubsets3(int N)
{
	boost::dynamic_bitset<> bits(N+1);
	vector<vector<int> > result;

	while (!bits[N])
	{
		for (size_t i = 0; i <= N; i ++)
		{
			if (!bits[i])
			{
				bits.set(i);
				break;
			}
			else
			{
				bits.reset(i);
			}
		}
		vector<int> subsets;
		for (size_t i = 0; i < N; i ++)
		{
			if (bits[i])
			{
				subsets.push_back(i);
			}
		}
		//bypass empty set
		if (!subsets.empty())
		{
			result.push_back(subsets);
		}
	}
	return result;
}



// assume N is smaller than 64
vector< vector<int> > SubsetsGenerator::generateSubsets2(int N)
{
	bitset<64> bits;
	vector<vector<int> > result;
	if (N > 63)
	{
		return result;
	}
	while (!bits[N])
	{
		for (size_t i = 0; i <= N; i ++)
		{
			if (!bits[i])
			{
				bits.set(i);
				break;
			}
			else
			{
				bits.reset(i);
			}
		}
		vector<int> subsets;
		for (size_t i = 0; i < N; i ++)
		{
			if (bits[i])
			{
				subsets.push_back(i);
			}
		}
		//bypass empty set
		if (!subsets.empty())
		{
			result.push_back(subsets);
		}
	}
	return result;
}




// assume N is smaller than 64
vector< vector<int> > SubsetsGenerator::generateSubsets(int N)
{
	uint64_t bits = 0;
	vector<vector<int> > result;
	if (N > 63)
	{
		return result;
	}
	for (size_t i = 0; i < (1<<N); i ++)
	{
		vector<int> subsets;
		for (size_t j = 0; j < N; j ++)
		{
			if (((i>>j) & 1) == 1)
			{
				subsets.push_back(j);
			}
		}
		//bypass empty set
		if (!subsets.empty())
		{
			result.push_back(subsets);
		}
	}
	return result;
}

ostream& operator <<(ostream& io, const vector<int>& vec)
{
	cout <<"[";
	for (size_t i = 0; i < vec.size(); i ++)
	{
		cout << vec[i];
		if (i < vec.size() - 1)
		{
			cout <<",";
		}
	}
	cout << "]" << endl;
}

void SubsetsGenerator::printSubsets(const vector<vector<int> >& subsets)
{
	cout<<"The total number of subset is: " << subsets.size() << endl;
	for (size_t i = 0; i < subsets.size(); i ++)
	{
		cout <<"{";
		for (size_t j = 0; j < subsets[i].size(); j ++)
		{
			cout << subsets[i][j];
			if (j < subsets[i].size() - 1)
			{
				cout <<",";
			}
		}
		cout << "}" << endl;
	}
}

void test1()
{
	SubsetsGenerator g;
	vector<vector<int> > subsets;
	clock_t start, end;
	size_t counter = 10000;
	start = clock();
	for (size_t i = 0; i < counter; i ++)
	{
		subsets = g.generateSubsets(6);
	}
	end = clock();
	cout << "difftime:" << (end - start) << endl;
	g.printSubsets(subsets);
	start = clock();
	for (size_t i = 0; i < counter; i ++)
	{
		subsets = g.generateSubsets2(6);
	}
	end = clock();
	cout << "difftime:" << (end - start) << endl;
	g.printSubsets(subsets);



	start = clock();
	for (size_t i = 0; i < counter; i ++)
	{
		subsets = g.generateSubsets3(6);
	}
	end = clock();
	cout << "difftime:" << (end - start) << endl;
	g.printSubsets(subsets);
}

void test2()
{
	SubsetsGenerator g;
	vector<vector<int> > subsets;
	clock_t start, end;
	size_t counter = 1;
	start = clock();
	for (size_t i = 0; i < counter; i ++)
	{
		subsets = g.generateSubsets3(24);
	}
	end = clock();
	cout << "difftime:" << (end - start) << endl;
	//g.printSubsets(subsets);
}






/*
 * test1 result:
 * difftime:1140000
The total number of subset is: 64
{}
{0}
{1}
{0,1}
{2}
{0,2}
{1,2}
{0,1,2}
{3}
{0,3}
{1,3}
{0,1,3}
{2,3}
{0,2,3}
{1,2,3}
{0,1,2,3}
{4}
{0,4}
{1,4}
{0,1,4}
{2,4}
{0,2,4}
{1,2,4}
{0,1,2,4}
{3,4}
{0,3,4}
{1,3,4}
{0,1,3,4}
{2,3,4}
{0,2,3,4}
{1,2,3,4}
{0,1,2,3,4}
{5}
{0,5}
{1,5}
{0,1,5}
{2,5}
{0,2,5}
{1,2,5}
{0,1,2,5}
{3,5}
{0,3,5}
{1,3,5}
{0,1,3,5}
{2,3,5}
{0,2,3,5}
{1,2,3,5}
{0,1,2,3,5}
{4,5}
{0,4,5}
{1,4,5}
{0,1,4,5}
{2,4,5}
{0,2,4,5}
{1,2,4,5}
{0,1,2,4,5}
{3,4,5}
{0,3,4,5}
{1,3,4,5}
{0,1,3,4,5}
{2,3,4,5}
{0,2,3,4,5}
{1,2,3,4,5}
{0,1,2,3,4,5}
difftime:1370000
The total number of subset is: 64
{0}
{1}
{0,1}
{2}
{0,2}
{1,2}
{0,1,2}
{3}
{0,3}
{1,3}
{0,1,3}
{2,3}
{0,2,3}
{1,2,3}
{0,1,2,3}
{4}
{0,4}
{1,4}
{0,1,4}
{2,4}
{0,2,4}
{1,2,4}
{0,1,2,4}
{3,4}
{0,3,4}
{1,3,4}
{0,1,3,4}
{2,3,4}
{0,2,3,4}
{1,2,3,4}
{0,1,2,3,4}
{5}
{0,5}
{1,5}
{0,1,5}
{2,5}
{0,2,5}
{1,2,5}
{0,1,2,5}
{3,5}
{0,3,5}
{1,3,5}
{0,1,3,5}
{2,3,5}
{0,2,3,5}
{1,2,3,5}
{0,1,2,3,5}
{4,5}
{0,4,5}
{1,4,5}
{0,1,4,5}
{2,4,5}
{0,2,4,5}
{1,2,4,5}
{0,1,2,4,5}
{3,4,5}
{0,3,4,5}
{1,3,4,5}
{0,1,3,4,5}
{2,3,4,5}
{0,2,3,4,5}
{1,2,3,4,5}
{0,1,2,3,4,5}
{}
difftime:1430000
The total number of subset is: 64
{0}
{1}
{0,1}
{2}
{0,2}
{1,2}
{0,1,2}
{3}
{0,3}
{1,3}
{0,1,3}
{2,3}
{0,2,3}
{1,2,3}
{0,1,2,3}
{4}
{0,4}
{1,4}
{0,1,4}
{2,4}
{0,2,4}
{1,2,4}
{0,1,2,4}
{3,4}
{0,3,4}
{1,3,4}
{0,1,3,4}
{2,3,4}
{0,2,3,4}
{1,2,3,4}
{0,1,2,3,4}
{5}
{0,5}
{1,5}
{0,1,5}
{2,5}
{0,2,5}
{1,2,5}
{0,1,2,5}
{3,5}
{0,3,5}
{1,3,5}
{0,1,3,5}
{2,3,5}
{0,2,3,5}
{1,2,3,5}
{0,1,2,3,5}
{4,5}
{0,4,5}
{1,4,5}
{0,1,4,5}
{2,4,5}
{0,2,4,5}
{1,2,4,5}
{0,1,2,4,5}
{3,4,5}
{0,3,4,5}
{1,3,4,5}
{0,1,3,4,5}
{2,3,4,5}
{0,2,3,4,5}
{1,2,3,4,5}
{0,1,2,3,4,5}
{}
 *
 */
