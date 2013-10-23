/*
 * MyAutoPtr.cpp
 *
 *  Created on: Oct 23, 2013
 *      Author: nick
 */

#include <iostream>
#include "MyAutoPtr.h"

using namespace std;

class Base
{
public:
	Base()
	{
		cout<<"Base constructor"<<endl;
	}
	virtual ~Base()
	{
		cout << "Base destructor" << endl;
	}
};

class Derived : public Base
{
public:
	Derived()
	{
		cout << "Derived constructor" << endl;
	}
	~Derived()
	{
		cout << "Derived destructor" << endl;
	}
	operator Base()
	{
		cout << "conversion operator from derived to base" <<endl;
		return *this;
	}
};

// a simple test for prime type
void test1()
{
	const size_t ArraySize = 100;

	MyAutoPtr<int> intPtr1(new int[ArraySize]);
	MyAutoPtr<int> intPtr2 = intPtr1;
	for (size_t i = 0; i < ArraySize; i ++)
	{
		intPtr2.get()[i] = i + 1;
	}
}

void test2()
{
	MyAutoPtr<Derived> derivedPtr(new Derived());
	MyAutoPtr<Base> basePtr(derivedPtr);
	//MyAutoPtr<Base> basePtr = derivedPtr;
	Derived derived;
	Base base;
	base = derived;

}

int main()
{

	test2();
	return 0;
}
