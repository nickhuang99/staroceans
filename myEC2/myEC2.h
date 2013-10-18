#ifndef MYEC2_H_INCLUDED
#define MYEC2_H_INCLUDED

#include <curl/curl.h>
#include <stdio.h>
#include <stdarg.h>
#include <map>
#include <set>
#include <vector>
#include <string.h>
#include <string>
#include <time.h>
#include <string>
#include <sstream>
#include <iostream>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/hmac.h>

#include "simplexml.h"

//using namespace std;

typedef std::map<std::string, std::string> StringMap;
typedef std::pair<std::string, std::string> StringPair;
typedef StringMap::const_iterator StringMap_Const_Iterator;
typedef std::set<std::string> StringSet;

typedef std::vector<StringPair> StringPairVector;


class MyEC2
{
    friend int mySimpleXmlCallback(const char *elementPath, const char *data, int dataLen, void *callbackData);
private:
    StringMap inputParam;
    StringSet outputParam;
    static char accessKey[32];
    static char secretAccessKey[64];
    static char keyPath[256];
    static bool isKeyOK;
    bool readKeyFile();
    void initialize(char* newKeyPath);
    std::string urlEncode(const std::string& aContent);
    std::string base64Encode(const char* aContent, size_t aContentSize);
    void encrypt(const char* strToSign, unsigned int inLength, char* strSigned, unsigned int* pOutLength);
    std::string getExpire();
    std::string getTimestamp();
    int curl_perform(const char* url);
    std::string calcQueryParam();
    std::string calcQuery();
    void calcSignature(std::string strQueryParam, char* buffer, int& bufferLength);
public:
    StringPairVector resultSet;
    MyEC2(char* newKeyPath);
    MyEC2();
    bool call(int number, ...);
    bool setParam(int number, ...);
    StringPairVector* getResult();
};



#endif // MYEC2_H_INCLUDED
