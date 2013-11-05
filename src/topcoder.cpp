//============================================================================
// Name        : topcoder.cpp
// Author      : nick
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================
/*
 * The following code is COPIED	BY ME from Topcoder as this genius under Topcoder account name "andbluer" writes such beautiful code that I
 * simply worship him! In order to fully understand the code, I decide to copy the code for myself.
 * 1. He is using "queue" to avoid recursition which I think my way failed in the first place. As this map is very large 501x501, recursion is
 * too deep in stack.
 * 2. He is very smart to take advantage of "visited" matrix to be used as "cost" therefore save quite some variables.
 * 3. However, I do have concerns. Originally he uses "memset(visited, -1, sizeof(visited));" I think this might have potential issues as memset
 * only set -1 for each "byte" so as for each integer it might not be -1?? (unless in 2's complement, each -1 in byte can be concatenated as -1
 * integer???? But this is purely by coincidence, not general for other number except 0, -1, right?).
 *
 */

/*
 * You are playing a video game that involves escaping from a dangerous area. Within the area there are DEADLY regions you can't enter, HARMFUL
 *  regions that take 1 life for every step you make in them, and NORMAL regions that don't affect you in any way. You will start from (0,0)
 *   and have to make it to (500,500) using only Up, Left, Right, and Down steps. The map will be given as a vector <string> deadly listing the
 *   DEADLY regions and a vector <string> harmful listing the HARMFUL regions. The elements in each of these parameters will be formatted as
 *   follows: Input format(quotes for clarity): "X1 Y1 X2 Y2" where (X1,Y1) is one corner of the region and (X2,Y2) is the other corner of the
 *   region The corners of the region are inclusive bounds (i.e. (4,1) and (2,2) include x-values between 4 and 2 inclusive and y-values
 *   between 1 and 2 inclusive). All unspecified regions are considered NORMAL. If regions overlap for a particular square, then whichever
 *   region is worst takes effect (e.g. DEADLY+HARMFUL = DEADLY, HARMFUL+NORMAL = HARMFUL, HARMFUL+HARMFUL = HARMFUL, DEADLY+NORMAL=DEADLY).
 *    Damage taken at each step occurs based on the destination square and not on the starting square (e.g. if the square (500,500) is HARMFUL
 *    you WILL take a point of damage stepping onto it; if the square (0,0) is HARMFUL you WON'T take a point of damage stepping off of it;
 *    this works analogously for DEADLY squares). Return the least amount of life you will have to lose in order to reach the destination.
 *    Return -1 if there is no path to the destination. Your character is not allowed to leave the map (i.e. have X or Y less than 0 or greater
 *     than 500)
 *
 *
 */
#include <functional>
#include <queue>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>

using namespace std;

#define N 501

struct Node
{
	int x, y, c;
	Node(int ax, int ay, int ac):x(ax), y(ay), c(ac){}
};

class Escape
{
private:
    int grid[501][501];
    int visited[501][501];
	void init(vector<string> harmful, vector<string> deadly)
	{
		int r, c;
		for (r = 0; r <= 500; r ++)
		{
			for (c = 0; c <= 500; c ++)
			{
				grid[r][c] = 0;
				visited[r][c] = -1;
			}
		}

		size_t i;
		for (i = 0; i < harmful.size(); i ++)
		{
			int x1, y1, x2, y2;
			stringstream ss(harmful[i]);
			ss >> x1 >> y1 >> x2 >> y2;
			if (x1 > x2) swap(x1, x2);
			if (y1 > y2) swap(y1, y2);
			for (r = y1; r <= y2; r++)
			{
				for (c = x1; c <= x2; c++)
				{
					grid[r][c] = 1;
				}
			}
		}
		for (i = 0; i < deadly.size(); i ++)
		{
			int x1, y1, x2, y2;
			stringstream ss(deadly[i]);
			ss >> x1 >> y1 >> x2 >> y2;
			if (x1 > x2) swap(x1, x2);
			if (y1 > y2) swap(y1, y2);
			for (r = y1; r <= y2; r++)
			{
				for (c = x1; c <= x2; c++)
				{
					grid[r][c] = -1;
				}
			}
		}
	}



public:
	int lowest(vector<string> harmful, vector<string> deadly)
	{
		init(harmful, deadly);
		queue<Node> q;
		q.push(Node(0,1,0));
		q.push(Node(1,0,0));
		while (!q.empty())
		{
			Node n = q.front();
			cout << "["<<n.x << "][" << n.y<<"]" <<endl;
			q.pop();
			if (n.x < 0 || n.x > 500 || n.y < 0 || n.y > 500 || grid[n.x][n.y] == -1)
			{
				continue;
			}
			int cost = n.c + grid[n.x][n.y];
			if (visited[n.x][n.y]!= -1 && cost >= visited[n.x][n.y])
			{
				continue;
			}
			visited[n.x][n.y] = cost;
			q.push(Node(n.x + 1, n.y, cost));
			q.push(Node(n.x - 1, n.y, cost));
			q.push(Node(n.x, n.y + 1, cost));
			q.push(Node(n.x, n.y - 1, cost));
		}
		return visited[500][500];
	}
};

void test1()
{
	Escape e;
	cout <<e.lowest(vector<string>(), vector<string>()) << endl;
}

int main()
{

	return 0;
}
