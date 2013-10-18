#include "myEC2.h"


bool MyEC2::isKeyOK = false;
char MyEC2::accessKey[32];
char MyEC2::secretAccessKey[64];
char MyEC2::keyPath[256];


MyEC2::MyEC2(char* newKeyPath)
{
    initialize(newKeyPath);
}
MyEC2::MyEC2()
{
    initialize("/home/nick/.ec2/s3key.txt");
}

void MyEC2::initialize(char* newKeyPath)
{
    if (!isKeyOK)
    {
        strcpy(keyPath, newKeyPath);
        isKeyOK = readKeyFile();
    }
}

bool MyEC2::readKeyFile()
{
    FILE* fd = fopen(keyPath, "r");
    if (fd)
    {
        fgets(accessKey, 32, fd);
        accessKey[strlen(accessKey) - 1]= '\0';
        fgets(secretAccessKey, 64, fd);
        secretAccessKey[strlen(secretAccessKey) - 1] = '\0';
        fclose(fd);
        return true;
    }
    return false;
}

std::string MyEC2::urlEncode(const std::string& aContent)
{
    std::string encoded;
    unsigned char c;
    unsigned char low, high;

    for (size_t i = 0; i < aContent.size(); i++)
    {
        c = aContent[i];
        if (isalnum(c)||(c == '-' || c == '_' || c == '.' || c == '~' || c == '+'))
        {
            encoded += c;
        }
        else
        {
            high = c / 16;
            low = c % 16;
            encoded += '%';
            encoded += (high < 10 ? '0' + high : 'A' + high - 10);
            encoded += (low < 10 ? '0' + low : 'A' + low - 10);
        }
    }
    return encoded;
}


std::string MyEC2::base64Encode(const char* aContent, size_t aContentSize)
{
    char* lEncodedString;
    long aBase64EncodedStringLength;
    // initialization for base64 encoding stuff
    BIO* lBio = BIO_new(BIO_s_mem());
    BIO* lB64 = BIO_new(BIO_f_base64());
    BIO_set_flags(lB64, BIO_FLAGS_BASE64_NO_NL);
    lBio = BIO_push(lB64, lBio);

    BIO_write(lBio, aContent, aContentSize);
    BIO_flush(lBio);
    aBase64EncodedStringLength = BIO_get_mem_data(lBio, &lEncodedString);
    // ensures null termination
    std::stringstream lTmp;
    lTmp.write(lEncodedString, aBase64EncodedStringLength);
    BIO_free_all(lBio);
    return lTmp.str(); // copy
}

void MyEC2::encrypt(const char* strToSign, unsigned int inLength, char* strSigned, unsigned int* pOutLength)
{
    HMAC_CTX theHctx;
    HMAC_CTX_init(&theHctx);

    HMAC_Init(&theHctx, secretAccessKey, strlen(secretAccessKey), EVP_sha1());
    HMAC(EVP_sha1(), secretAccessKey, strlen(secretAccessKey), (const unsigned char *) strToSign, inLength,
        (unsigned char*)strSigned, pOutLength);
    HMAC_CTX_cleanup(&theHctx);
}

bool MyEC2::call(int pairNumber, ...)
{
    if (!isKeyOK)
    {
        return false;
    }
    va_list vl;
    va_start(vl, pairNumber);
    inputParam.clear();
    for (int i = 0; i < pairNumber; i ++)
    {
        char* strKey = va_arg(vl, char*);
        char* strValue = va_arg(vl, char*);
        inputParam.insert(StringPair(strKey, strValue));
    }
    va_end(vl);
    inputParam.insert(StringPair("SignatureMethod", "HmacSHA1"));
    inputParam.insert(StringPair("SignatureVersion", "2"));
    inputParam.insert(StringPair("AWSAccessKeyId", accessKey));
    inputParam.insert(StringPair("Expires", getExpire()));
    inputParam.insert(StringPair("Version", "2010-11-15"));

    resultSet.clear();
    //...
    std::string url = calcQuery();
    curl_perform(url.c_str());
    outputParam.clear();
    return true;
}

bool MyEC2::setParam(int number, ...)
{
    if (!isKeyOK)
    {
        return false;
    }
    va_list vl;
    va_start(vl, number);
    inputParam.clear();
    for (int i = 0; i < number; i ++)
    {
        char* strValue = va_arg(vl, char*);
        outputParam.insert(std::string(strValue));
    }
    va_end(vl);
    return true;
}

StringPairVector* MyEC2::getResult()
{
    return &resultSet;
}

std::string MyEC2::getExpire()
{
    time_t after = time(NULL) + 300;
    char buffer[32];
    strftime(buffer, 32, "%FT%H:%M:%SZ", gmtime(&after));
    std::string in = buffer;
    return urlEncode(in);
}

std::string MyEC2::getTimestamp()
{
    time_t after = time(NULL);
    char buffer[32];
    strftime(buffer, 32, "%FT%H:%M:%SZ", gmtime(&after));
    std::string in = buffer;
    return urlEncode(in);
}


size_t writeHeader( void *ptr, size_t size, size_t nmemb, void *user)
{
    MyEC2* my= (MyEC2*)user;
    //printf("size = %d, nmemb=%d\n", size, nmemb);
    char* str = (char*)ptr;
    char* sub = strstr(str, ": ");
    if (sub == NULL)
    {
        // actually the first line is NOT a header, i.e. "http 403 forbidden"
        my->resultSet.push_back(StringPair("status", str));
    }
    else
    {
        char header[32];
        char body[256];
        int len;
        len = sub - str;
        memcpy(header, str, len);
        header[len] = '\0';
        // remove the trailing \r\n
        len = size * nmemb - len - 2 - 2;
        memcpy(body, sub + 2, len);
        body[len] = '\0';
        my->resultSet.push_back(StringPair(header, body));
    }
    return size * nmemb;
}

size_t myCurlCallback(void *ptr, size_t size, size_t nmemb, void *user)
{
    SimpleXml*simple = (SimpleXml*)user;
    char* data = (char*) ptr;
    size_t result = size*nmemb;
    if (simplexml_add(simple, data, result))
    {
        return 0;
    }
    else
    {
        return result;
    }
}

int mySimpleXmlCallback(const char *elementPath, const char *data, int dataLen, void *callbackData)
{
    char buffer[1024];
    MyEC2* my = (MyEC2*) callbackData;
    const char* sub;
    sub = strrchr((char*)elementPath, '/');
    if (sub == NULL)
    {
        sub = elementPath;
    }
    else
    {
        // skip "/"
        sub ++;
    }

    if (my->outputParam.find(std::string(sub))!= my->outputParam.end())
    {
        bool isWhiteSpace = true;
        for (int i = 0; i < dataLen; i ++)
        {
            if (data[i] != ' ' && data[i] != '\t' && data[i] != '\n')
            {
                isWhiteSpace = false;
            }
            buffer[i] = data[i];
        }

        buffer[dataLen] = '\0';
        if (!isWhiteSpace)
        {
            //printf("\n*******************************\nelementPath:%s\t\tdata[%d]=%s\n\n", elementPath, dataLen, buffer);
            my->resultSet.push_back(StringPair(sub, buffer));
        }
    }
    return 0;
}

int MyEC2::curl_perform(const char* url)
{
    //FILE* stream = fopen("mytest.txt", "w");
    SimpleXml simple;
    simplexml_initialize(&simple, mySimpleXmlCallback, this);

    CURL* pCurl = curl_easy_init( );
    curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &simple);
    curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, myCurlCallback);
    curl_easy_setopt(pCurl, CURLOPT_HEADERFUNCTION, writeHeader);
    curl_easy_setopt(pCurl, CURLOPT_WRITEHEADER, this);

        //CURLOPT_ENCODINGMozilla/4.0 (Compatible; %s; libs3 %s.%s; %s)userAgentInfo, LIBS3_VER_MAJOR, LIBS3_VER_MINOR, platform);
    struct curl_slist *slist=NULL;

    slist = curl_slist_append(slist, "pragma:");
    slist = curl_slist_append(slist, "Accept-Encoding: identity");
    slist = curl_slist_append(slist, "Transfer-Encoding:");
    slist = curl_slist_append(slist, "Content-Type: text/html;charset=UTF-8");
    slist = curl_slist_append(slist, "Accept:");
    slist = curl_slist_append(slist, "User-Agent: Mozilla/4.0 (Compatible; ubuntu32; myEC2 1.0; Linux)");
    slist = curl_slist_append(slist, "Connection: Keep-Alive" );
    if (slist != NULL)
    {
        curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, slist);
    }
    curl_easy_setopt(pCurl, CURLOPT_URL, url);

    curl_easy_perform(pCurl);

    curl_easy_cleanup(pCurl);
    curl_slist_free_all(slist); /* free the list again */
    simplexml_deinitialize(&simple);
    return 0;
    //fclose(stream);
}

std::string MyEC2::calcQueryParam()
{
    std::stringstream strStream;
    bool isFirst = true;
    for (StringMap::iterator it = inputParam.begin(); it != inputParam.end(); it ++)
    {
        if (isFirst)
        {
            isFirst = false;
        }
        else
        {
            strStream<<"&";
        }
        strStream << it->first<<"="<<it->second;
    }
    return strStream.str();
}

void MyEC2::calcSignature(std::string strQueryParam, char* buffer, int& bufferLength)
{
    std::stringstream strStream;
    //char* GetString= "GET\nec2.amazonaws.com\n/\n";
    //strStream << "GET"<<endl<<"ec2.amazonaws.com"<<endl<<"/"<<endl;
    strStream <<"GET\nec2.amazonaws.com\n/\n";
    strStream << strQueryParam;
    std::string str2sign = strStream.str();
    char strSigned[1024];
    unsigned int length = 0;
    encrypt(str2sign.c_str(), str2sign.size(), strSigned, &length);
    std::string str = urlEncode(base64Encode(strSigned, length));
    strcpy(buffer, str.c_str());
    bufferLength = str.length();
}

std::string MyEC2::calcQuery()
{
    std::stringstream strStream;
    strStream << "http://ec2.amazonaws.com/?";
    std::string strQueryParam = calcQueryParam();
    strStream<<strQueryParam;

    char buffer[1024];
    int bufferLength = 0;
    calcSignature(strQueryParam, buffer, bufferLength);
    strStream<<"&Signature="<<buffer;
    return strStream.str();
}




/*
// testing...
void outputHeader()
{
    for (StringMap_Iterator it = stringMap.begin(); it != stringMap.end(); it ++)
    {
        cout<<"key:"<<it->first<<";value:"<<it->second<<endl;
    }
}
*/

