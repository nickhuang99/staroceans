/*
 * fraction.cpp
 *
 *  Created on: Nov 28, 2013
 *      Author: nick
 *      Problem Statement
    
Given a fraction, you can divide its numerator and denominator by one of their common divisors to get a fraction with the same value. For example, 8/16 can become 4/8 if you divide the numerator and denominator by 2. With some fractions, you can get a similar result by removing digits that are common to both the numerator and denominator. The removed digits must occur in the same relative order in both the numerator and denominator of the original fraction. For example, consider 4784/7475. We can remove the 7 in the numerator and the first 7 in the denominator, and then remove the second 4 in the numerator and the 4 in the denominator to get 48/75, which has the same value as the original fraction. You are given the numerator and denominator of a fraction. Use the procedure described above to get a fraction with the smallest possible numerator. Return the resulting fraction formatted as "NUMERATOR/DENOMINATOR" (quotes for clarity only), where both the numerator and denominator are written without leading zeroes. If the procedure cannot be applied, return the original fraction.
Definition
    
Class:
AnomalousCancellation
Method:
reducedFraction
Parameters:
int, int
Returns:
string
Method signature:
string reducedFraction(int numerator, int denominator)
(be sure your method is public)
    

Constraints
-
numerator and denominator will each be between 10 and 9999, inclusive.
Examples
0)

    
16
64
Returns: "1/4"
We can cancel the 6's to get 16/64=1/4.
1)

    
4784
7475
Returns: "48/75"
This is the example from the problem statement.
2)

    
25
25
Returns: "2/2"
The fraction can be reduced to 2/2 or 5/5. We choose 2/2 since it has the smallest numerator.
3)

    
95
19
Returns: "5/1"
We can cancel the 9's to get 95/19=5/1.
4)

    
100
1000
Returns: "1/10"
We can cancel two 0's to get 100/1000=1/10.
5)

    
123
456
Returns: "123/456"
The numerator and denominator have no digits in common, so the cancellation procedure can't be applied.
6)

    
151
1057
Returns: "1/7"
After cancelling the 1's and the 5's we get 151/1057=1/07, and we write the denominator without the leading zero.
 */


#include <vector>
#include <string>
#include <sstream>
#include <iostream>

using namespace std;

class AnomalousCancellation
{
private:
	int gcd(int num, int den)
	{
		/*
		function gcd(a, b)
    while b ≠ 0
       t := b
       b := a mod b
       a := t
    return a

    	while (den != 0)
    	{
    		int tmp = den;
    		den = num % den;
    		num = tmp;
    	}
    	*/
    	/*
    	function gcd(a, b)
    while a ≠ b
        if a > b
           a := a − b
        else
           b := b − a
    return a
    */
    	while (num != den)
    	{
    		if (num > den)
    		{
    			num = num - den;
    		}
    		else
    		{
    			den = den - num;
    		}
    	}
    	return num;
    }

public:
	string int2string(int i)
	{
		stringstream ss;
		ss << i;
		return ss.str();
	}
	bool stringDiff(const string& src, const string& dst, string& result)
	{
		size_t i = 0, j = 0;
		while (i < src.size() && j < dst.size())
		{
			if (src[i] == dst[j])
			{
				i ++;
				j ++;
			}
			else
			{
				result.push_back(src[i]);
				i ++;
			}
		}
		if (j == dst.size())
		{
			result.append(src.substr(i, string::npos));
			return true;
		}
		return false;
	}

	string reducedFraction(int numerator, int denominator)
	{
		string numString = int2string(numerator);
		string denString = int2string(denominator);
		int gcdInt = gcd(numerator, denominator);
		int div = gcdInt;

		while (div > 1)
		{
			int num = numerator/div;
			int den = denominator/div;
			string numStr = int2string(num);
			string denStr = int2string(den);
			string numDiff;
			string denDiff;
			if (stringDiff(numString, numStr, numDiff) && stringDiff(denString, denStr, denDiff))
			{
				if (numDiff.compare(denDiff) == 0)
				{
					return numStr + "/" + denStr;
				}
			}
			div --;
			while (gcdInt % div != 0)
			{
				div --;
			}
		}

		return numString + "/" + denString;
	}
};

int main()
{
	AnomalousCancellation a;
	cout << a.reducedFraction(4784, 7475) << endl;
}
