/*
Problem Statement
    
You are working on setting up a new filing system, and need to know which k-th letter occurs most frequently among the names of your clients. Given vector <string> names, each element of which contains the name of a client, and int k, the 1-based index of the character to examine, return a string indicating the k-th letter that appears most often. Ignore any names containing less than k characters. If multiple k-th letters appear the same number of times, return the one among them that comes first alphabetically.
Definition
    
Class:
NamesList
Method:
popularInitial
Parameters:
vector <string>, int
Returns:
string
Method signature:
string popularInitial(vector <string> names, int k)
(be sure your method is public)
    

Constraints
-
names will contain between 1 and 50 elements, inclusive.
-
Each element of names will contain between 1 and 50 uppercase letters ('A'-'Z'), inclusive.
-
k will be between 1 and 50, inclusive.
-
At least one element of names will contain at least k characters.
Examples
0)

    
{"ANNE", "BILL", "BOB"}
1
Returns: "B"
Here, we just want the first character. There is 1 'A' and 2 'B's.
*/

#include <vector>
#include <string>

using namespace std;

class NamesList
{
public:
	string popularInitial(vector<string> names, int k)
	{
		int freq[26] = {0};
		for (size_t i = 0; i < names.size(); i ++)
		{
			if (names[i].size() >= k)
			{
				freq[names[i][k-1] - 'A'] ++;
			}
		}
		int counter = 0;
		char resultIndex = 0;
		for (size_t i = 0; i < 26; i ++)
		{
			if (freq[i] > counter)
			{
				resultIndex = i;
				counter = freq[i];
			}
		}
		string result(1, resultIndex+'A');
		return result;
	}
};
