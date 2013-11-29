/*

Problem Statement
    
The product value of a string is the product of all the digits ('0'-'9') in the string. For example, the product value of "123" is 1 * 2 * 3 = 6. A string is called colorful if it contains only digits and the product value of each of its nonempty contiguous substrings is distinct.  For example, the string "263" has six substrings: "2", "6", "3", "26", "63" and "263". The product values of these substrings are: 2, 6, 3, 2 * 6 = 12, 6 * 3 = 18 and 2 * 6 * 3 = 36, respectively. Since all six product values are distinct, "263" is colorful. On the other hand, "236" is not colorful because two of its substrings, "6" and "23", have the same product value (6 = 2 * 3).  Return the k-th (1-based) lexicographically smallest colorful string of length n. If there are less than k colorful strings of length n, return an empty string instead.
Definition
    
Class:
ColorfulStrings
Method:
getKth
Parameters:
int, int
Returns:
string
Method signature:
string getKth(int n, int k)
(be sure your method is public)
    

Notes
-
The lexicographically smaller of two strings is the one that has the smaller character ('0' < '1' < ... < '9') at the first position where they differ.
Constraints
-
n will be between 1 and 50, inclusive.
-
k will be between 1 and 1,000,000,000, inclusive.
Examples
0)

    
3
4
Returns: "238"
The first four smallest colorful strings of length 3 are "234", "235", "237" and "238".
1)

    
4
2000
Returns: ""
The number of colorful strings of length 4 is less than 2000.
2)

    
5
1
Returns: "23457"

3)

    
2
22
Returns: "52"

4)

    
6
464
Returns: "257936"

This problem statement is the exclusive and proprietary property of TopCoder, Inc. Any unauthorized use or reproduction of this information without the prior written consent of TopCoder, Inc. is strictly prohibited. (c)2003, TopCoder, Inc. All rights reserved.
*/
#include <string>
#include <sstream>
#include <set>

using namespace std;

class ColorfulStrings
{
private:
	int string2int(const string& str)
	{
		stringstream ss;
		ss.str(str);
		int result;
		ss >> result;
		return result;
	}
	int findMax(int n)
	{
		string str;
		for (size_t i = 0; i < n; i ++)
		{
			str.push_back('9');
		}
		return string2int(str);
	}
	int findMin(int n)
	{
		string str;
		str.push_back('1');
		for (size_t i = 1; i < n; i ++)
		{
			str.push_back('0');
		}
		return string2int(str);
	}
			
	string int2string(int num)
	{
		stringstream ss;
		ss << num;
		return ss.str();
	}
	int stringProduct(const string& str)
	{
		int result = 1;
		for (size_t i = 0; i < str.size(); i ++)
		{
			result *= str[i]-'0';
		}
		return result;
	}
	bool isColorful(int num)
	{
		set<int> intSet;
		string str = int2string(num);
		for (size_t i = 0; i < str.size(); i ++)
		{
			for (size_t j = i + 1; j <= str.size(); j ++)
			{
				string sub = str.substr(i, j-i);
				int product = stringProduct(sub);
				if (intSet.find(product) == intSet.end())
				{
					intSet.insert(product);
				}
				else
				{
					return false;
				}
			}
		}
		return true;
	}
		
	
public:
	string getKth(int n, int k)
	{
		int maxInt = findMax(n);
		int minInt = findMin(n);
		int counter = 0;
		for (int i = minInt; i <= maxInt; i ++)
		{
			if (isColorful(i))
			{
				counter ++;
			}
			if (counter == k)
			{
				return int2string(i);
			}
		}
		return string();
	}
};
				
	
