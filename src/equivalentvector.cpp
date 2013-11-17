/*
 * equivalentvector.cpp
 *
 *  Created on: Nov 17, 2013
 *      Author: nick
 *      equivalent sequence is a coding problem in topcoder which is merely 300 points, however, I feel it is so complex that I
 *       spent more than a whole day to figure out some solution, which I believe. The definition of equivalent is more or less
 *       a 'weak order of sum of its all subsets'. i.e. given two equal length sequence or vector, for all subsets of the
 *       sequence, the sum should be at same rank.
 *       given a vector of int as vec, let's define 'ind' as a vector of index of vector. define sum(ind) as the sum of
 *       vec element with index contained in  'ind'.
 *
 */

#include <vector>
#include <cstdint>
#include <algorithm>
#include "powerset.h"

using namespace std;

class SubsetsGenerator;

class EquivalentVector
{
private:
	SubsetsGenerator generator;
	int subsetSum(const vector<int>& vec, const vector<int>& index)
	{
		int result = 0;
		for (size_t i = 0; i < index.size(); i ++)
		{
			result += vec[index[i]];
		}
		return result;
	}
	vector<int>  calcSubsetSum(const vector<int>& vec, const vector<vector<int> >& indexSet)
	{
		vector<int> result;
		for (size_t i = 0; i < indexSet.size(); i ++)
		{
			result.push_back(subsetSum(vec, indexSet[i]));
		}

		return result;
	}
	vector<int> calcRankVector(const vector<int>& vec)
	{
		vector<int> replic(vec.begin(), vec.end());
		sort(replic.begin(), replic.end());
		vector<int> result;
		for (size_t i = 0; i < vec.size(); i ++)
		{
			pair<vector<int>::iterator, vector<int>::iterator > rangePair;
			rangePair = equal_range(replic.begin(), replic.end(), vec[i]);
			result.push_back(distance(replic.begin(), rangePair.first));
			result.push_back(distance(replic.begin(), rangePair.second));
		}
		return result;
	}

public:
	bool is_equivalent(const vector<int>& left, const vector<int>& right)
	{
		vector< vector<int> > subsets = generator.generateSubsets(left.size());

		generator.printSubsets(subsets);
		vector<int> leftSubsetSum = calcSubsetSum(left, subsets);
		cout << "leftSubsetSum:" << leftSubsetSum << endl;

		vector<int> rightSubsetSum = calcSubsetSum(right, subsets);
		cout << "rightSubsetSum:" << rightSubsetSum << endl;
		vector<int> leftRank = calcRankVector(leftSubsetSum);
		cout << "leftRank" << leftRank <<endl;
		vector<int> rightRank = calcRankVector(rightSubsetSum);

		cout << "rightRank" << rightRank <<endl;
		return equal(leftRank.begin(), leftRank.end(), rightRank.begin());
	}
};

int main()
{
	EquivalentVector eq;
	vector<int> left = {4, 1, 3, 4};
	vector<int> right = {5, 3, 4, 5};
	cout << left << endl;
	cout << right << endl;
	cout << eq.is_equivalent(left, right) << endl;
	return 0;
}
