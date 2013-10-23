/*
 * MyAutoPtr.h
 *
 *  Created on: Oct 20, 2013
 *      Author: nick
 */

#ifndef MYAUTOPTR_H_
#define MYAUTOPTR_H_

//#include <tr1/shared_ptr.h>
//#include <memory>

using namespace std;

template<class T>
class MyAutoPtrRef
{
private:
	T* m_ptr;
public:
	explicit MyAutoPtrRef(T* ptr): m_ptr(ptr){}
};

template <class T>
class MyAutoPtr
{
private:
	T* m_ptr;
public:

	// default constructor?
	explicit MyAutoPtr(T* ptr=0):m_ptr(ptr){}
	MyAutoPtr(MyAutoPtr& other): m_ptr(other.release()){}

	// constructor with compatible T1
	template<class T1>
	MyAutoPtr(MyAutoPtr<T1>& other): m_ptr(other.release()){}

	MyAutoPtr(MyAutoPtrRef<T> ref): m_ptr(ref.m_ptr){}


	// nothing special for destructor
	~MyAutoPtr(){delete m_ptr;}

	// internal helper function
	T* release()
	{
		T* tmp = m_ptr;
		m_ptr = 0;
		return tmp;
	}
	void reset(T* aPtr)
	{
		if (aPtr != m_ptr)
		{
			delete m_ptr;
			m_ptr = aPtr;
		}
	}
	// access functions
	T* get()
	{
		return m_ptr;
	}
	T& operator*()
	{
		return *m_ptr;
	}
	T* operator->()
	{
		return m_ptr;
	}
	// assignment operator
	MyAutoPtr& operator=(MyAutoPtr& other)
	{
		reset(other.release());
		return *this;
	}

	template<class T1>
	MyAutoPtr& operator=(MyAutoPtr<T1>& other)
	{
		reset(other.release());
		return *this;
	}

	MyAutoPtr& operator=(MyAutoPtrRef<T>& ref)
	{
		if (m_ptr != ref.m_ptr)
		{
			delete m_ptr;
			m_ptr = ref.m_ptr;
		}
		return *this;
	}

	// conversion operator
	template<class T1>
	operator MyAutoPtrRef<T1>()
	{
		return MyAutoPtrRef<T1>(release());
	}

	// conversion operator for compatible T1
	template <class T1>
	operator MyAutoPtr<T1>()
	{
		return MyAutoPtr<T1>(release());
	}

/*
	template <>
	class MyAutoPtr<void>
	{

	};
	*/
};


#endif /* MYAUTOPTR_H_ */
