/*

Problem Statement
    
You have just entered the Colorful Maze. You are given the layout of the maze in the vector <string> maze, where the j-th character of the i-th element is the cell at row i, column j. The following types of cells exist in the maze:
'#' - Wall. You cannot enter these cells.
'.' - Empty cell. You can walk freely into these cells.
'A'-'G' - Empty cell with a colored floor. Each of 'A'-'G' represents a different color. You can walk into these cells.
'$' - Entrance. This is the cell in which you start.
'!' - Exit. This is the cell you must reach to finish the maze.
 Colored floors with certain colors are dangerous, but you don't know upfront which colors are dangerous. You only know the probability of each color being dangerous. You are given a vector <int> trap, where the first element is the percent probability of color 'A' being dangerous, the second element is the percent probability of color 'B' being dangerous, and so on. When you step into a cell with a dangerous color, you get damaged by a trap.  When you're inside the maze, you can only move in the four cardinal directions. You cannot move diagonally. Return the probability that you can finish the maze without getting damaged, assuming that you walk according to a strategy that maximizes this probability. See examples for further clarification.
Definition
    
Class:
ColorfulMazeTwo
Method:
getProbability
Parameters:
vector <string>, vector <int>
Returns:
double
Method signature:
double getProbability(vector <string> maze, vector <int> trap)
(be sure your method is public)
    

Notes
-
The returned value must have an absolute or relative error less than 1e-9.
Constraints
-
maze will contain between 1 and 50 elements, inclusive.
-
Each element of maze will contain between 1 and 50 characters, inclusive.
-
Each element of maze will contain the same number of characters.
-
Each character in maze will be one of '#', '.', 'A', 'B', 'C', 'D', 'E', 'F', 'G', '$', and '!'.
-
maze will contain exactly one '$' and exactly one '!'.
-
trap will contain exactly 7 elements.
-
Each element of trap will be between 0 and 100, inclusive.
Examples
0)

    
{ ".$.",
  "A#B",
  "A#B",
  ".!." }
{ 50, 50, 0, 0, 0, 0, 0 }
Returns: 0.5
First, go left one cell, and then down one cell into the 'A'. One of two things might happen with equal probability:
You get damaged. You fail to finish the maze.
You do not get damaged. This means all cells with color 'A' are safe, so you can continue through the 'A' zone and get to the exit.
The probability of reaching the exit is 0 (the first case) + 0.5 (the second case) = 0.5. The probability is the same if you go through the 'B' zone.
1)

    
{ ".$.",
  "A#B",
  "A#B",
  ".!." }
{ 50, 40, 0, 0, 0, 0, 0 }
Returns: 0.6
As 'B' is safer than it was in the previous example, it is better to go through the 'B' zone.
2)

    
{ "$A#",
  ".#.",
  "#B!" }
{ 10, 10, 10, 10, 10, 10, 10 }
Returns: 0.0
No matter how you walk, you cannot reach the exit, so you should return 0.
3)

    
{ "$A..",
  "##.A",
  "..B!" }
{ 50, 50, 0, 0, 0, 0, 0 }
Returns: 0.5

4)

    
{ "$C..",
  "##.A",
  "..B!" }
{ 50, 50, 100, 0, 0, 0, 0 }
Returns: 0.0

5)

    
{ ".$.D.E.F.G.!." }
{ 10, 20, 30, 40, 50, 60, 70 }
Returns: 0.036000000000000004

This problem statement is the exclusive and proprietary property of TopCoder, Inc. Any unauthorized use or reproduction of this information without the prior written consent of TopCoder, Inc. is strictly prohibited. (c)2003, TopCoder, Inc. All rights reserved.
*/

#include <vector>
#include <string>
#include <set>
#include <queue>

using namespace std;

struct Coord
{	
	Coord():row(0), col(0){;}
	int row, col;
	Coord up()
	{
		Coord ret;
		ret.row = row+1;
		ret.col = col;
		return ret;
	}
	Coord down()
	{
		Coord ret;
		ret.row = row-1;
		ret.col = col;
		return ret;
	}
	Coord left()
	{
		Coord ret;
		ret.row = row;
		ret.col = col-1;
		return ret;
	}
	Coord right()
	{
		Coord ret;
		ret.row = row;
		ret.col = col+1;
		return ret;
	}		
	bool operator ==(const Coord& other) const
	{
		return row == other.row && col == other.col;
	}	
	Coord& operator=(const Coord& other)
	{
		row = other.row;
		col = other.col;
		return *this;
	}
};

struct CoordComp
{
	bool operator()(const Coord& left, const Coord& right)const
	{
		return (left.row<right.row) || (left.row==right.row&&left.col <right.col);
	}
};

typedef set<Coord, CoordComp> CoordSet;

struct Node
{
	Coord current;
	CoordSet route;
};
typedef queue<Node> NodeQueue;

bool getCoord(const vector<string>& maze, const Coord& coord, char& ch)
{
	if (coord.row >= 0 && coord.col >= 0 && coord.row < maze.size() && coord.col < maze[0].size())
	{
		ch = maze[coord.row][coord.col];
		return true;
	}
 	return false;
}

class ColorfulMazeTwo
{
private:
	bool findCoord(const vector<string>& maze, char ch, Coord& coord)
	{
		for (size_t r = 0; r < maze.size(); r ++)
		{
			for (size_t c = 0; c < maze[0].size(); c ++)
			{
				if (maze[r][c] == ch)
				{
					coord.row = r;
					coord.col = c;
					return true;
				}
			}
		}
		return false;
	}
	bool isValid(const vector<string>& maze, const Coord& next)
	{
		char ch;
		if (getCoord(maze, next, ch))
		{
			if (ch == '#')
			{
				return false;
			}
			return true;
		}
		return false;
	}
		
	void addNode(const vector<string>& maze, NodeQueue& q, const Node& node, const Coord& next)
	{
		if (isValid(maze, next))
		{
			if (node.route.find(next) == node.route.end())
			{
				Node newNode(node);
				newNode.route.insert(node.current);
				newNode.current=next;
				q.push(newNode);
			}
		}			
	}
	double calculateRoute(const vector<string>& maze, const vector<int>& trap, const Node& node)
	{
		double result = 1.0;
		bool boolArray[7]={false};
		for (CoordSet::const_iterator it = node.route.begin(); it != node.route.end(); it ++)
		{
			char ch;
			getCoord(maze, *it, ch);
			if (ch >= 'A' && ch <= 'G')
			{
				boolArray[ch-'A'] = true;
			}
		}
		for (int i = 0; i < 7; i ++)
		{
			if (boolArray[i])
			{
				result *= (double)(100 - trap[i])/100.0;
			}
		}
		return result;
	}				
			
public:
	double getProbability(vector<string> maze, vector<int> trap)
	{
		Coord start, end;
		findCoord(maze, '$', start);
		findCoord(maze, '!', end);
		Node node;
		node.current=start;
		double result = 0.0;
		NodeQueue q;
		q.push(node);
		while (!q.empty())
		{
			node = q.front();
			q.pop();
			if (node.current == end)
			{
				double ret = calculateRoute(maze, trap, node);
				if (ret > result)
				{
					result = ret;
				}
			}
			Coord next;
			next = node.current.up();
			addNode(maze, q, node, next);
			next = node.current.down();
			addNode(maze, q, node, next);
			next = node.current.left();
			addNode(maze, q, node, next);
			next = node.current.right();
			addNode(maze, q, node, next);
			
		}
		return result;
	}
};
				


				

