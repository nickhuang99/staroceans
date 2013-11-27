/*
 * paperGame.cpp
 *
 *  Created on: Nov 27, 2013
 *      Author: nick
 *      roblem Statement
    
You are playing a game that starts with a rectangular piece of grid paper. In a move, you cut all remaining pieces of paper with a single cut horizontally or vertically, along a grid line. Thus, if you start the game with a 5 x 6 piece of paper, after the first move you might have a 2 x 6 and a 3 x 6 paper. For the second move, you would then have to divide each of those two pieces.
As a twist to make the game challenging, there is a forbidden size, and at no point in the game play may the width or height of any remaining piece equal the forbidden size (including the initial position).
If, at any point in the game, there are 1 x 1 pieces of paper, they are removed from play. The game is over when no more pieces remain to be cut. You are given ints height and width describing the size of the paper initially, and int forbidden, the forbidden paper size. Return the maximum number of turns you can take before all of the paper has been cut. If it is impossible to play the game through to the end without having a forbidden size, return -1.
Definition
    
Class:
PaperGame
Method:
mostMoves
Parameters:
int, int, int
Returns:
int
Method signature:
int mostMoves(int width, int height, int forbidden)
(be sure your method is public)
    

Constraints
-
width will be between 1 and 50, inclusive.
-
height will be between 1 and 50, inclusive.
-
forbidden will be between 2 and 50, inclusive.
Examples
0)

    
2
2
3
Returns: 2
With a 2x2 sheet of paper, the height or width can never be 3, so we don't need to worry. In fact, there is only one move we can make, to divide the paper into two 1x2 sheets. For our second move, we again only have one choice, to divide each of those into 1x1 sheets. Thus, we can make two moves.
1)

    
4
1
3
Returns: 2
Here, we could potentially cut the sheet into two 2x1 sheets, or a 1x1 and a 3x1. However, since 3 is a forbidden size, we are forced to first cut the sheet into two 2x1 pieces. Then, we can make one more move.
2)

    
5
4
3
Returns: 5
A bigger board, again we need to be careful to avoid creating a forbidden size.
 */

// I give up debugging as I don't really understand the problem. It is more or less an English problem with regarding interpretation of question. What is stopping condition?

#include <queue>
#include <iostream>

using namespace std;

struct Node
{
int width, height;
int depth;
};

class PaperGame
{
public:
	int mostMoves(int width, int height, int forbidden)
	{
		int result = -1;
		Node node;
		node.width = width;
		node.height = height;
		node.depth = 0;
		queue<Node> q;
		q.push(node);
		while (!q.empty())
		{
			Node n = q.front();
			q.pop();
			bool bNext = false;
			bool bEnd = false;

			for (int w = 1; w <= n.width/2; w ++)
			{
				Node left, right;
				left.width = n.width - w;
				right.width = w;
				left.height = right.height = n.height;
				left.depth = right.depth = n.depth + 1;

				if (left.width*left.height != forbidden && right.width*right.height != forbidden)
				{
					if (left.width*left.height ==1 && right.width*right.height == 1)
					{
						bEnd = true;
					}
					q.push(left);
					q.push(right);
					bNext = true;
				}
			}
			for (int h = 1; h < n.height; h ++)
			{
				Node top, bottom;
				top.height = h;
				bottom.height = n.height - h;
				top.width = bottom.width = n.width;
				top.depth = bottom.depth = n.depth + 1;
				if (top.width*top.height != forbidden && bottom.width*bottom.height != forbidden)
				{
					if (top.width*top.height == 1 && bottom.width*bottom.height == 1)
					{
						bEnd = true;
					}
					q.push(top);
					q.push(bottom);
					bNext = true;
				}
			}
			if (bNext && bEnd && n.depth+1 > result)
			{
				result = n.depth + 1;
			}
		}
		return result;
	}
};


int main()
{
	PaperGame p;
	cout << p.mostMoves(5, 4, 3) << endl;

}
