/*
Given a vector <int> values containing positive integers return (quotes for clarity):
"ASCENDING mean" if the numbers are in increasing order and there are no repeated values,
"DESCENDING mean" if the numbers are in decreasing order and there are no repeated values,
"NONASCENDING freq" if the numbers are in decreasing order and contain repeated values,
"NONDESCENDING freq" if the numbers are in increasing order and contain repeated values,
or "NOTHING" if the numbers are none of the above.
where mean is a reduced fraction representing the average of the numbers formatted as (quotes for clarity) "numerator/denominator" and freq is the number of times the most frequently occurring value occurred in the sequence. Neither numerator nor denominator should have any leading zeros. For example (quotes for clarity):
values = {1,2,4,11}       return "ASCENDING 9/2" since the average is 18/4 = 9/2
values = {1,2,2,2,3,4}   return "NONDESCENDING 3"  since 2 occurred 3 times
values = {6,5,1}         return "DESCENDING 4/1" since the average is 12/3 = 4/1
values = {5,5,4,4,1}     return "NONASCENDING 2" since 5 occurred twice
values = {1,2,3,4,1}     return "NOTHING" since no other choice is possible
*/


#include <vector>
#include <string>
#include <map>
#include <sstream>

using namespace std;

class Ordered
{
private:
	map<int, int> freq;	
	int gcd(int a, int b)
	{
		if (b == 1)
		{
			return 1;
		}
  		int c = a % b;
  		while(c != 0)
  		{
    		a = b;
    		b = c;
    		c = a % b;
  		}
  		return b;
	}
public:
	string getType(vector<int> values)
	{
		int sum = 0;
		
		int occur = 0;
		// 0 ASCENDING, 
		// 1 NONASCENDING
		// 2 DESCENDING
		// 3 NONDESCENDING
		// 4 NOTHING
		int resultType = -1;
		
		sum = values[0];
		bool bRepeated = false;
		freq[values[0]] = 1;
		
		for (size_t i = 1; i < values.size(); i ++)
		{
			sum += values[i];
			
			int ret = values[i-1] - values[i];
			if ( i == 1)
			{
				resultType = ret;
			}
			else
			{
				if ((ret < 0 && resultType > 0) || (ret > 0 && resultType < 0))
				{
					return "NOTHING";
				}
				if (ret == 0)
				{
					bRepeated = true;
				}
				else
				{
					resultType = ret;
				}
			}
			if (freq.find(values[i])!= freq.end())
			{
				freq[values[i]] = freq[values[i]] + 1;
				if (freq[values[i]] > occur)
				{
					occur = freq[values[i]];
				}
			}
			else
			{
				freq[values[i]] = 1;
			}
		}
		stringstream ss;
		if (!bRepeated)
		{
			int den = gcd(sum, values.size());
			if (resultType > 0)
			{
				ss << "DESCENDING ";  
			}
			else
			{
				ss << "ASCENDING ";
			}
			ss << sum/den <<"/" << values.size()/den;
		}
		else
		{
			if (resultType > 0)
			{
				ss << "NONASCENDING ";
			}
			else
			{
				ss << "NONDESCENDING ";
			}
			ss << occur;
		}
		return ss.str();
	}
};
		
