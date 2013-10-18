
#include "UtilityManager.h"
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
using namespace std;
using namespace MyS3MultiPartUploader;

unsigned char ch2hex(unsigned char ch)
{
	if (ch > 9)
	{
		return ch - 10 + 'A';
	}
	else
	{
		return ch + '0';
	}
}

string convertToHex(const string& strIn)
{
	string str;
	const unsigned char MASKHI = 0xF0;
	const unsigned char MASKLO = 0x0F;
	for (int i = 0; i < strIn.size(); i ++)
	{
		unsigned char hi = (MASKHI & strIn[i]) >> 4;
		unsigned char lo = MASKLO & strIn[i];
		str.push_back(ch2hex(hi));
		str.push_back(ch2hex(lo));
	}
	return str;
}

MyMD5::MyMD5(bool bHexOutput)
{
	bHex = bHexOutput;
	MD5_Init(&ctx);
}

void MyMD5::update(const string& strIn)
{
	MD5_Update(&ctx, strIn.c_str(), strIn.size());
}

string MyMD5::finalize()
{
	unsigned char raw_digest[16];
	MD5_Final(raw_digest, &ctx);

	if (bHex)
	{
		return convertToHex(string((char*)raw_digest, 16));
	}
	return string((char*)raw_digest, 16);
}

MyBase64Encoder::MyBase64Encoder()
{
	b64 = BIO_new(BIO_f_base64());
	bio = BIO_new( BIO_s_mem() );
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	bio = BIO_push(b64, bio);
	strEncoded.clear();
}

void MyBase64Encoder::update(const string& strIn)
{
	BIO_write(bio, strIn.c_str(), strIn.size());
	BIO_flush(bio);
	char* ptrOut = NULL;
	size_t rLen = BIO_get_mem_data(bio, &ptrOut);
	if (ptrOut)
	{
		strEncoded.append(ptrOut, rLen);
	}

}
string MyBase64Encoder::finalize()
{
	BIO_free_all(bio);
	return strEncoded;
}


MyBase64Decoder::MyBase64Decoder()
{
	b64 = BIO_new(BIO_f_base64());
	strDecoded.clear();
}

void MyBase64Decoder::update(const string& strIn)
{
	size_t nTotalToDecode = strIn.size(), nTotalDecoded = 0, nDecoded = 0, nToDecode = 0;
	while (nTotalToDecode > 0)
	{
		if (nTotalToDecode > MyBase64Decoder_Internal_Buffer_Size)
		{
			nToDecode = MyBase64Decoder_Internal_Buffer_Size;
		}
		else
		{
			nToDecode = nTotalToDecode;
		}
		memset(buffer, 0, nToDecode);
		bmem = BIO_new_mem_buf((void*)(strIn.c_str() + nTotalDecoded), nToDecode);
		bmem = BIO_push(b64, bmem);
		nDecoded = BIO_read(bmem, buffer, nToDecode);
		strDecoded.append((char*)buffer, nDecoded);

		nTotalToDecode -= nToDecode;
		nTotalDecoded += nToDecode;
	}
}

string MyBase64Decoder::finalize()
{
	BIO_free_all(bmem);
	return strDecoded;
}

MySH1::MySH1(const string& strKeyIn, bool bHexOutput)
{
	bHex = bHexOutput;
	strKey = strKeyIn;
	HMAC_CTX_init(&theHctx);
	HMAC_Init(&theHctx, strKey.c_str(), strKey.size(), EVP_sha1());
	strSigned.clear();
}

void MySH1::update(const string& strIn)
{
	size_t nTotalToSign = strIn.size(), nTotalSigned = 0, nToSign = 0;
	unsigned int nSigned = 0;
	while (nTotalToSign > 0)
	{
		if (nTotalToSign > MySH1_Internal_Buffer_Size)
		{
			nToSign = MySH1_Internal_Buffer_Size;
		}
		else
		{
			nToSign = nTotalToSign;
		}
		HMAC(EVP_sha1(), strKey.c_str(), strKey.size(), (unsigned char*)(strIn.c_str() + nTotalSigned), nToSign, buffer, &nSigned);
		strSigned.append((char*)buffer, nSigned);
		nTotalSigned += nToSign;
		nTotalToSign -= nToSign;
	}
}

string MySH1::finalize()
{
	HMAC_CTX_cleanup(&theHctx);
	if (bHex)
	{
		return convertToHex(strSigned);
	}
	return strSigned;
}

/*
TraverseHandler MyDir::setFileHandler(TraverseHandler newFileHandler)
{
	TraverseHandler temp = fileHandler;
	fileHandler = newFileHandler;
	return temp;
}

TraverseHandler MyDir::setDirHandler(TraverseHandler newDirHandler)
{
	TraverseHandler temp = dirHandler;
	dirHandler = newDirHandler;
	return temp;
}


bool MyDir::doFile(const string& filePath, StringMap& stringMap, const string& prefix)
{
	//cout<<filePath.file_string()<<endl;
	//cout<<filePath.filename()<<endl;
	string str = prefix + "/" + urlEncode(filePath);

	if (stringMap.insert(make_pair<string, string>(filePath, str)) == stringMap.end())
	{
		//cout<<"error"<<endl;
	}
}

bool MyDir::doDir(const path& thePath)
{
	directory_iterator it(thePath), dirEnd;
	while (it != dirEnd)
	{
		//cout<< it->filename()<<endl;
		if (is_directory(*it))
		{
			//cout<< it->filename()<<endl;
			// check if it has white space in between.
			if (dirHandler == NULL || dirHandler(thePath.filename(), it->filename())
			{
				doDir(*it);
			}
		}
		else
		{
			if (is_regular_file(*it))
			{
				//string str = urlEncode(it->filename());
				if (fileHandler == NULL || fileHandler(thePath.filename(), it->filename())
				{
					doFile(it->filename(), stringMap, prefix);
				}
			}
			else
			{
				//cout<<"file is not regular:"<<it->filename()<<endl;
			}
		}
		it ++;
	}
}

void MyDir::doTraverseDir(const string& rootDir)
{
	directory_iterator it(rootDir), dirEnd;
	while (it != dirEnd)
	{
		if (is_directory(*it))
		{
			//cout<< it->filename()<<endl;
			// check if it has white space in between.
			if (dirHandler == NULL || dirHandler(thePath.filename(), it->filename())
			{
				doDir(*it);
			}
		}
		else
		{
			if (is_regular_file(*it))
			{
				//string str = urlEncode(it->filename());
				if (fileHandler == NULL || fileHandler(thePath.filename(), it->filename())
				{
					doFile(it->filename(), stringMap, prefix);
				}
			}
			else
			{
				//cout<<"file is not regular:"<<it->filename()<<endl;
			}
		}
		it ++;
	}
}

bool MyDir::traverseDir(const string& rootDir)
{
	if (is_directory(rootDir))
	{
		doTraverseDir(rootDir);
		return true;
	}
	return false;
}
*/


bool ci_char_traits::eq(char c1, char c2)
{
    return toupper(c1) == toupper(c2);
}
bool ci_char_traits::ne(char c1, char c2)
{
    return toupper(c1) != toupper(c2);
}
bool ci_char_traits::lt(char c1, char c2)
{
    return toupper(c1) <  toupper(c2);
}


int ci_char_traits::compare(const char* s1, const char* s2, size_t n)
{
    while ( n-- != 0 )
    {
        if ( toupper(*s1) < toupper(*s2) ) return -1;
        if ( toupper(*s1) > toupper(*s2) ) return 1;
        ++s1;
        ++s2;
    }
    return 0;
}

const char* ci_char_traits::find(const char* s, int n, char a)
{
    while ( n-- > 0 && toupper(*s) != toupper(a) )
    {
        ++s;
    }
    return s;
}

string StringManager::int2string(int number)
{
    stringstream stream;
    stream << number;
    return stream.str();
}

int StringManager::icompare(const std::string& left, const std::string& right)
{
    if (left.size() != right.size())
    {
        return left.size() - right.size();
    }
    else
    {
        for (int i = 0; i < left.size(); i ++)
        {
            char l = toupper(left[i]), r = toupper(right[i]);

            if ( l != r )
            {
                return l - r;
            }
        }
        return 0;
    }
}

int StringManager::incompare(const std::string& left, const std::string& right, size_t len)
{
    int length = len;
    if (length > left.size())
    {
        length = left.size();
    }
    if (length > right.size())
    {
        length = right.size();
    }

    for (int i = 0; i < length; i ++)
    {
        char l = toupper(left[i]), r = toupper(right[i]);
        if ( l != r )
        {
            return l - r;
        }
    }
    return 0;
}


bool StringManager::string2int(const std::string& str, int& number)
{

    return sscanf(str.c_str(), "%d", &number) == 1;
}

    /*
    stringstream stream(str);
    return (stream << number);
    */
