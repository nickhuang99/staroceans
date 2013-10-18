#ifndef UTILITYMANAGER_H_INCLUDED
#define UTILITYMANAGER_H_INCLUDED

#include "stdafx.h"
#include <openssl/md5.h>
#include <openssl/hmac.h>


namespace MyS3MultiPartUploader
{
#define MySH1_Internal_Buffer_Size 256
#define MyBase64Decoder_Internal_Buffer_Size 256

	class MyBase64Encoder
	{
	private:
		string strEncoded;
		BIO *bio;
        BIO *b64;
	public:
		MyBase64Encoder();
		void update(const string& strIn);
		string finalize();
	};


	class MyBase64Decoder
	{
	private:
		string strDecoded;
        BIO* b64;
        BIO* bmem;
        unsigned char buffer[MyBase64Decoder_Internal_Buffer_Size];
	public:
		MyBase64Decoder();
		void update(const string& strIn);
		string finalize();
	};

	class MySH1
    {
	private:
		HMAC_CTX theHctx;
		string strKey;
		bool bHex;
		string strSigned;
		unsigned char buffer[MySH1_Internal_Buffer_Size];
	public:
		MySH1(const string& strKeyIn, bool bHexOutput = false);
		void update(const string& strIn);
		string finalize();
    };

    class MyMD5
    {
    private:
        MD5_CTX ctx;
        bool bHex;
    public:
        MyMD5(bool bHexOutput = true);
        void update(const string& strIn);
		string finalize();
    };

	class MyDir
	{
	private:

	public:
		MyDir(const string& rootDir){;}
		virtual bool fileHandler(const string& dirName, const string& fileName){return true;}
		virtual bool dirHandler(const string& parentDir, const string& dirName){return true;}
	};



}

#endif // UTILITYMANAGER_H_INCLUDED
