#ifndef MYEC2_H_INCLUDED
#define MYEC2_H_INCLUDED

#include "stdafx.h"
#include <curl/curl.h>
#include <pthread.h>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;

namespace MyS3MultiPartUploader
{
	struct MultiPartDesc;
	struct PartDesc
	{
		size_t partId;
		string md5;
		string eTag;
		size_t offset;
		size_t current;
		size_t size;
		FILE* stream;
		bool finished;
		//MultiPartDesc* pMulti;
	};

	//typedef map<size_t, PartDesc> IndexPartDescMap;
	typedef vector<PartDesc> PartDescVector;

	struct MultiPartDesc
	{
		string bucket;
		string object;
		string file;
		string uploadId;
		size_t partCount;
		string contentType;
		PartDescVector partDescVector;
		string eTag;
		pthread_mutex_t mutex;
		size_t nThreadNumber;
		string accessKey;
        string secretAccessKey;
	};

#define MYS3_FILE_BUFFER_SIZE 1024
    class MyS3
    {
        friend int mySimpleXmlCallback(const char *elementPath, const char *data, int dataLen, void *callbackData);
    private:
		static char* HttpVerb[3];

        static char accessKey[32];
        static char secretAccessKey[64];
        static char keyPath[256];
        static bool isKeyOK;

        //char ip[32];
        MultiPartDesc multiPartDesc;
		string getFileContentType(const string& fileName, FILE* stream);

		void addDefaultHttpHeader(curl_slist*& slist, const string& strTimestamp, const string& strSignature);
        void initializeKey(const char* newKeyPath);
        bool initialize(const string& bucketName, const string& objectName, const string& fileName, size_t nSuggestedPartNumber,
			size_t nThreadNumber);
		bool uploadMultiPart();
		bool doUploadMultiPart(int index);
        bool readKeyFile();
        bool initMultiPartUpload();

        bool completeMultiPartUpload();
        string createCompletePartBody();

        int doDir(const path& subDir, const string& bucket, const string& object, bool bUsingHashName);
		int doFile(const string& fileName, const string& bucket, const string& object, bool bUsingHashName);
		string calculateFileMD5Name(const string& fileName);
    public:
        MyS3(char* newKeyPath = NULL);
        StringPairVector getAllUploadId(const string& bucket);
        bool abortUpload(const string& bucket, const string& object, const string& uploadId);
        bool multiPartUpload(const string& bucketName, const string& objectName, const string& fileName,
			size_t nSuggestedPartNumber, size_t nThreadNumber);
		bool uploadDir(const string& bucket, const string& subObject, const string& dir, bool bUsingHashName = false);
		bool uploadFile(const string& bucket, const string& object, const string& fileName, bool bUsingHashName = false);
    };



}


#endif // MYEC2_H_INCLUDED
