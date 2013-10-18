
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

