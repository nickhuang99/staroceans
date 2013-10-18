#ifndef STDAFX_H_INCLUDED
#define STDAFX_H_INCLUDED

#include <stdio.h>
#include <map>
#include <set>
#include <vector>
#include <string.h>
#include <string>
#include <time.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include "log.h"
#include "ExceptionManager.h"


namespace MyS3MultiPartUploader
{
    typedef std::map<std::string, std::string> StringMap;
    typedef std::map<std::string, StringMap> StringMapMap;
    typedef std::pair<std::string, std::string> StringStringPair;
    typedef std::pair<std::string, StringMap> StringStringMapPair;
    typedef std::set<std::string> StringSet;
    typedef std::set<int> IntSet;
    typedef std::vector<std::string> StringVector;
    typedef std::vector<StringStringPair> StringPairVector;


    typedef std::map<int, std::string> IntStringMap;
}


#endif // STDAFX_H_INCLUDED
