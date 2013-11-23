//============================================================================
// Name        : dllclient.cpp
// Author      : nick
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
using namespace std;

extern "C" void foo();

int main()
{
	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	foo();
	cout << endl << "do you see which dll is linked?" << endl;
	return 0;
}
