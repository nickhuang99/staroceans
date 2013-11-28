/*
 * fraction.cpp
 *
 *  Created on: Nov 28, 2013
 *      Author: nick
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
