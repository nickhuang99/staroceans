/*
 * nines.cpp
 *
 *  Created on: Dec 6, 2013
 *      Author: nick
 *      Actually this is NOT topcoder problem, but my own problem.
 *      1. In Samsung Note 8.0, the opening lock is using a "pattern" for security instead of password. You need to find out
 *      total number of possible patterns.
 *      2.
 *      *  *  *
 *      *  *  *
 *      *  *  *
 *
 *      0  1  2
 *      3  4  5
 *      6  7  8
 *
 */

#include <vector>
#include <string>
#include <iostream>

using namespace std;

class Pattern
{
private:
	bool isValid(char start, char end)
	{
		if (start == '0' && end =='8' || start=='1' && end=='7' || start=='2' && end=='6' || start =='3' && end == '5'
				|| start =='5'&& end =='3' || start =='6'&&end=='2' || start == '7'&&end=='1' || start=='8'&&end=='0')
		{
			return false;
		}
		return true;
	}
	bool noRepeat(const string& path, char ch)
	{
		for (size_t i = 0; i < path.size(); i ++ )
		{
			if (ch == path[i])
			{
				return false;
			}
		}
		return true;
	}

	int findPattern(const string& path)
	{
		int result = 1;
		char last = path[path.size()-1];
		for (char ch = '0'; ch <= '8'; ch ++)
		{
			if (ch != last && isValid(last, ch) && noRepeat(path, ch))
			{
				result += findPattern(path+ch);
			}
		}
		return result;
	}
	void doCollectPattern(const string& path, vector<string>& result)
	{
		char last = path[path.size()-1];
		for (char ch = '0'; ch <= '8'; ch ++)
		{
			if (ch != last && isValid(last, ch) && noRepeat(path, ch))
			{
				result.push_back(path+ch);
				doCollectPattern(path+ch, result);
			}
		}
	}
public:
	int totalPatterns()
	{
		int result = 0;
		for (char ch = '0'; ch <= '8'; ch ++)
		{
			result += findPattern(string(1, ch));
		}
		return result;
	}
	vector<string> collectPatterns()
	{
		vector<string> result;
		for (char ch = '0'; ch <= '8'; ch ++)
		{
			// we avoid the pattern of length == 1
			doCollectPattern(string(1, ch), result);
		}
		return result;
	}
	void drawPattern(const string& path)
	{
		char matrix[9] = {'*','*','*','*','*','*','*','*','*',};
		for (size_t i = 0; i < path.size(); i ++)
		{
			matrix[path[i]-'0'] = i+'0';
		}
		cout << "==================" << endl;
		for (int r = 0; r < 3; r ++)
		{
			for (int c = 0; c < 3; c ++)
			{
				cout << matrix[r*3+c] << "  ";
			}
			cout << endl;
		}
		cout << "==================" << endl;
	}
};

int main()
{
	Pattern p;
	cout << p.totalPatterns() << endl;
	vector<string> patterns;
	patterns = p.collectPatterns();
	for (size_t i = 0; i < patterns.size(); i ++)
	{
		cout << patterns[i] << endl;
		p.drawPattern(patterns[i]);
	}
	return 0;
}
