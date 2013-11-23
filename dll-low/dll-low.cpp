/*
 * dll-low.cpp
 *
 *  Created on: Nov 23, 2013
 *      Author: nick
 */

#include <cstdio>


using namespace std;

extern "C" void foo()
{
	printf("I am dll low\n");
}
