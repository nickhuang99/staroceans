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

/*
	typedef bool (*TraverseHandler)(const string& parentName, const string& sonName);
	class MyDir
	{
	private:
		TraverseHandler dirHandler;
		TraverseHandler fileHandler;
	public:
		MyDir():dirHandler(NULL), fileHandler(NULL){;}
		TraverseHandler setFileHandler(TraverseHandler newFileHandler);
		TraverseHandler setDirHandler(TraverseHandler newDirHandler);
		bool traverseDir(const string& rootDir);
	};
*/
	class StringManager
    {
    public:
        static StringVector split(const std::string& src, char* delimiter=" \t\n");
        static std::string trim(const std::string& src, char* delimiter=" \t\n");
        static std::string int2string(int num);
        static bool string2int(const std::string& str, int& num);
        static int icompare(const std::string& left, const std::string& right);
        static int incompare(const std::string& left, const std::string& right, size_t len);
    };

    struct ci_char_traits : public std::char_traits<char>
    {
        static bool eq(char c1, char c2);

        static bool ne(char c1, char c2);

        static bool lt(char c1, char c2);

        static int compare(const char* s1, const char* s2, size_t n);

        static const char* find(const char* s, int n, char a);
    };

    typedef std::basic_string<char, ci_char_traits> ci_string;


}

#endif // UTILITYMANAGER_H_INCLUDED
