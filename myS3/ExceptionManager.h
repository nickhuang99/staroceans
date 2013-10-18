#ifndef EXCEPTIONMANAGER_H_INCLUDED
#define EXCEPTIONMANAGER_H_INCLUDED

#include <stdexcept>

using namespace std;

class XmlException : public runtime_error
{
public:
    XmlException(string& msg):runtime_error(msg){}
};

class Ec2Exception : public runtime_error
{
public:
    Ec2Exception(string& msg):runtime_error(msg){}
};

#endif // EXCEPTIONMANAGER_H_INCLUDED
