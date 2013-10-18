#include "myS3.h"
#include "UtilityManager.h"
#include <curl/curl.h>
#include <openssl/evp.h>
#include <sys/socket.h>       /* for AF_INET */
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/hmac.h>
#include "simplexml.h"


using namespace MyS3MultiPartUploader;

char* MyS3::HttpVerb[3] = {"GET", "PUT", "POST"};
bool MyS3::isKeyOK = false;
char MyS3::accessKey[32];
char MyS3::secretAccessKey[64];
char MyS3::keyPath[256];

int mySimpleXmlCallback(const char *elementPath, const char *data, int dataLen, void *callbackData);
size_t myCurlCallback(void *ptr, size_t size, size_t nmemb, void *user);
size_t writeHeader( void *ptr, size_t size, size_t nmemb, void *user);
int myDebugFunc(CURL * pCurl, curl_infotype, char* ptrData , size_t nDataSize, void * userData);
string getTimestamp();
string doSignature(const string& strToSign, const string& strKey);
string int2string(int number);
string calculateMD5(FILE* stream, size_t offset, size_t size);
size_t myReadFunc(void *ptr, size_t size, size_t nmemb, void *userdata);
int mySimpleXmlGetUploadIdCallback(const char *elementPath, const char *data, int dataLen, void *callbackData);

void* uploadMultiPartThreadFunc(void* arg)
{
	MultiPartDesc* pMulti = (MultiPartDesc*)arg;
	MultiPartDesc& multiPartDesc = *pMulti;
	FILE* stream = NULL;
	do
	{
		pthread_mutex_lock(&pMulti->mutex);
		int index = -1;
		for (int i = 0; i < pMulti->partDescVector.size(); i ++)
		{
			if (!pMulti->partDescVector[i].finished)
			{
				pMulti->partDescVector[i].finished = true;
				index = i;
				break;
			}
		}
		pthread_mutex_unlock(&pMulti->mutex);
		if (index == -1)
		{
			//no more job, quit
			break;
		}

		string str;
		StringMap stringMap;
		string strToSign, strUrl, strSubUrl, strCustomRequest;
		string strPutRequest;
		StringMap headerMap;
		bool bRetry = false;
		PartDesc& partDesc = multiPartDesc.partDescVector[index];

		if (stream == NULL)
		{
			stream = fopen(pMulti->file.c_str(), "rb");
			if (stream == NULL)
			{
				partDesc.finished = false;
				// error
				printf("cannot open file!\n");
				return NULL;
			}
		}
		partDesc.stream = stream;

		if (partDesc.md5.size() == 0)
		{
			partDesc.md5 = calculateMD5(stream, partDesc.offset, partDesc.size);
		}
		printf("[debug]:==> thread about to upload part %d\n", index);
		do
		{
			partDesc.current = partDesc.offset;
			fseek(partDesc.stream, partDesc.offset, SEEK_SET);

			string strTimestamp = getTimestamp();
			//PUT /ObjectName?partNumber=PartNumber&uploadId=UploadId
			strPutRequest = "/" + multiPartDesc.object + "?partNumber=" + int2string(index + 1) + "&uploadId=" + multiPartDesc.uploadId;

			FILE_LOG(logDEBUG)<<"[strPutRequest]"<< strPutRequest;

			strUrl = "http://s3.amazonaws.com/" + multiPartDesc.bucket ;
			FILE_LOG(logDEBUG)<<"[strUrl]"<< strUrl;
			strToSign = "PUT\n";
			strToSign += partDesc.md5 + "\n";
			strToSign += multiPartDesc.contentType + "\n";
			strToSign += strTimestamp + "\n";
			strToSign += "/" + multiPartDesc.bucket + strPutRequest;

			FILE_LOG(logDEBUG)<<"[strToSign]"<< strToSign;

			string strSignature = doSignature(strToSign, pMulti->secretAccessKey);

			FILE_LOG(logDEBUG)<<"[strSignature]"<< strSignature;
			SimpleXml simple;
			simplexml_initialize(&simple, mySimpleXmlCallback, &stringMap);

			CURL* pCurl = curl_easy_init();

			curl_easy_setopt(pCurl, CURLOPT_UPLOAD, 1L);

			curl_easy_setopt(pCurl, CURLOPT_READFUNCTION, myReadFunc);
			curl_easy_setopt(pCurl, CURLOPT_READDATA, &partDesc);

			strCustomRequest = "PUT " + strPutRequest;
			curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, strCustomRequest.c_str());
			FILE_LOG(logDEBUG)<<"[strPutRequest]"<< strPutRequest;

			curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);
			curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &simple);
			curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, myCurlCallback);
			curl_easy_setopt(pCurl, CURLOPT_HEADERFUNCTION, writeHeader);
			curl_easy_setopt(pCurl, CURLOPT_WRITEHEADER, &headerMap);

			curl_easy_setopt(pCurl, CURLOPT_DEBUGFUNCTION, myDebugFunc);

			curl_easy_setopt(pCurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
			struct curl_slist *slist=NULL;

			str = "Host: " + multiPartDesc.bucket  + ".s3.amazonaws.com";
			slist = curl_slist_append(slist, str.c_str());

			str = "Date: " + strTimestamp;
			FILE_LOG(logDEBUG)<<"[Date]"<< str;
			slist = curl_slist_append(slist, str.c_str());

			str = "Authorization: AWS ";
			str += pMulti->accessKey;
			str += ":";
			str += strSignature;
			FILE_LOG(logDEBUG)<<"[Authorization]"<< str;
			slist = curl_slist_append(slist, str.c_str());

			slist = curl_slist_append(slist, "Accept-Encoding: identity");

			slist = curl_slist_append(slist, "Transfer-Encoding: ");
			slist = curl_slist_append(slist, "User-Agent: Mozilla/4.0 (Compatible; ubuntu32; MyS3 1.0; Linux)");
			slist = curl_slist_append(slist, "Expect: ");
			str = "Content-MD5: " + partDesc.md5;
			slist = curl_slist_append(slist, str.c_str());

			str = "Content-Length: " + int2string(partDesc.size);
			slist = curl_slist_append(slist, str.c_str());

			str = "Content-Type: " + multiPartDesc.contentType;
			slist = curl_slist_append(slist, str.c_str());

			if (slist != NULL)
			{
				curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, slist);
			}
			curl_easy_setopt(pCurl, CURLOPT_URL, strUrl.c_str());

			curl_easy_perform(pCurl);

			curl_easy_cleanup(pCurl);
			curl_slist_free_all(slist); /* free the list again */
			simplexml_deinitialize(&simple);

			partDesc.eTag = headerMap["ETag"];
			FILE_LOG(logDEBUG)<<"[ETag]"<< partDesc.eTag;
			if (partDesc.eTag.size() == 0)
			{
				bRetry = true;
				printf("[debug]:==> thread upload part %d failed and about to retry\n", index);
			}
			else
			{
				bRetry = false;
			}
		} while (bRetry);
	}
	while (true);
	fclose(stream);
	return NULL;
}


string int2string(int number)
{
    stringstream stream;
    stream << number;
    return stream.str();
}


MyS3::MyS3(char* newKeyPath)
{
	if (newKeyPath == NULL)
	{
		newKeyPath = "/home/nick/.ec2/s3key.txt";
	}
	initializeKey(newKeyPath);
}

void MyS3::initializeKey(const char* newKeyPath)
{
	if (!isKeyOK)
	{
		strcpy(keyPath, newKeyPath);
		isKeyOK = readKeyFile();
	}
}

bool MyS3::readKeyFile()
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


bool MyS3::multiPartUpload(const string& bucketName, const string& objectName, const string& fileName, size_t nSuggestedPartNumber,
	size_t nThreadNumber, const string& strMime)
{
	bool bResult = false;
	if (initialize(bucketName, objectName, fileName, nSuggestedPartNumber, nThreadNumber, strMime))
	{
//#define NICK_DEBUG 1
#ifdef NICK_DEBUG
		FILE*stream = fopen(multiPartDesc.file.c_str(), "rb");
		for (int i = 0; i < 6; i ++)
		{
			PartDesc& partDesc = multiPartDesc.partDescVector[i];
			string md5 = calculateMD5(stream, partDesc.offset, partDesc.size);
			printf("md5=%s\n", md5.c_str());
		}
#else
		if (initMultiPartUpload())
		{
			int i;
			pthread_t* pThreads = new pthread_t[multiPartDesc.nThreadNumber];
			if (pThreads != NULL)
			{
				for (i = 0; i < multiPartDesc.nThreadNumber; i ++)
				{
					pthread_create(pThreads + i, NULL, uploadMultiPartThreadFunc, &multiPartDesc);
				}
				for ( i = 0; i < multiPartDesc.nThreadNumber; i ++)
				{
					pthread_join(pThreads[i], NULL);
				}
				delete[] pThreads;
				if (completeMultiPartUpload())
				{
					bResult = true;
				}
			}
			//fclose(multiPartDesc.stream);
		}
#endif
	}
	return bResult;
}

string calculateMD5(FILE* stream, size_t offset, size_t size)
{
	char buffer[MYS3_FILE_BUFFER_SIZE];
	MyMD5 myMD5(false);
	size_t totalSize, readSize, toReadSize;
	totalSize = size;
	fseek(stream, offset, SEEK_SET);
	while (totalSize > 0)
	{
		toReadSize = MYS3_FILE_BUFFER_SIZE;
		if (toReadSize > totalSize)
		{
			toReadSize = totalSize;
		}
		readSize = fread(buffer, 1, toReadSize, stream);
		string str(buffer, readSize);
		myMD5.update(str);
		totalSize -= readSize;
	}
	MyBase64Encoder myBase64Encoder;
	myBase64Encoder.update(myMD5.finalize());
	return myBase64Encoder.finalize();
}

void MyS3::addFileContentType(FILE* stream)
{
	multiPartDesc.contentType = "video/avi";
}

bool MyS3::abortUpload(const string& bucket, const string& object, const string& uploadId)
{
	string str;
	StringMap stringMap, headerMap;

	string strToSign, strDeleteRequest, strUrl, strSubUrl, strCustomRequest, strSignature;
	string strTimestamp = getTimestamp();

	strDeleteRequest = "/" + object + "?uploadId=" + uploadId;
	strSubUrl = "/" + bucket + strDeleteRequest;

	FILE_LOG(logDEBUG)<<"[strDeleteRequest]"<< strDeleteRequest;
	//strSubUrl = "/" + bucket + strDeleteRequest;

	strUrl = "http://s3.amazonaws.com/" + bucket ;
	FILE_LOG(logDEBUG)<<"[strUrl]"<< strUrl;
	strToSign = "DELETE\n\ntext/html\n";
	strToSign += strTimestamp + "\n";

	strToSign += strSubUrl;

	FILE_LOG(logDEBUG)<<"[strToSign]"<< strToSign;

	doSignature(strToSign, string(secretAccessKey));

	strSignature = doSignature(strToSign, secretAccessKey);

	FILE_LOG(logDEBUG)<<"[strSignature]"<< strSignature;

	SimpleXml simple;
	simplexml_initialize(&simple, mySimpleXmlCallback, &stringMap);

	CURL* pCurl = curl_easy_init();

	strCustomRequest = "DELETE " + strDeleteRequest;
	curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, strCustomRequest.c_str());

	curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &simple);
	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, myCurlCallback);
	curl_easy_setopt(pCurl, CURLOPT_HEADERFUNCTION, writeHeader);
	curl_easy_setopt(pCurl, CURLOPT_WRITEHEADER, &headerMap);

	curl_easy_setopt(pCurl, CURLOPT_DEBUGFUNCTION, myDebugFunc);

	curl_easy_setopt(pCurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
	struct curl_slist *slist=NULL;

	str = "Host: " + bucket  + ".s3.amazonaws.com";
	slist = curl_slist_append(slist, str.c_str());

	str = "Date: " + strTimestamp;
	FILE_LOG(logDEBUG)<<"[Date]"<< str;
	slist = curl_slist_append(slist, str.c_str());

	str = "Authorization: AWS ";
	str += accessKey;
	str += ":";
	str += strSignature;
	FILE_LOG(logDEBUG)<<"[Authorization]"<< str;
	slist = curl_slist_append(slist, str.c_str());

	slist = curl_slist_append(slist, "Accept-Encoding: identity");

	slist = curl_slist_append(slist, "Transfer-Encoding: ");
	slist = curl_slist_append(slist, "User-Agent: Mozilla/4.0 (Compatible; ubuntu32; MyS3 1.0; Linux)");

	slist = curl_slist_append(slist, "Content-Type: text/html");

	if (slist != NULL)
	{
		curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, slist);
	}
	curl_easy_setopt(pCurl, CURLOPT_URL, strUrl.c_str());

	curl_easy_perform(pCurl);

	curl_easy_cleanup(pCurl);
	curl_slist_free_all(slist); /* free the list again */
	simplexml_deinitialize(&simple);

	return true;

}

StringPairVector MyS3::getAllUploadId(const string& bucket)
{
	string str;
	//StringVector stringVector;
	StringPairVector stringPairVector;

	StringMap headerMap;

	string strToSign, strGetRequest, strUrl, strSubUrl, strCustomRequest, strSignature;
	string strTimestamp = getTimestamp();

	strGetRequest = "/?uploads";

	FILE_LOG(logDEBUG)<<"[strGetRequest]"<< strGetRequest;
	strSubUrl = "/" + bucket + strGetRequest;

	strUrl = "http://s3.amazonaws.com/" + bucket ;
	FILE_LOG(logDEBUG)<<"[strUrl]"<< strUrl;
	strToSign = "GET\n\ntext/html\n";
	strToSign += strTimestamp;
	strToSign += "\n";
	strToSign += strSubUrl;

	FILE_LOG(logDEBUG)<<"[strToSign]"<< strToSign;

	doSignature(strToSign, string(secretAccessKey));

	strSignature = doSignature(strToSign, secretAccessKey);

	FILE_LOG(logDEBUG)<<"[strSignature]"<< strSignature;

	SimpleXml simple;
	simplexml_initialize(&simple, mySimpleXmlGetUploadIdCallback, &stringPairVector);

	CURL* pCurl = curl_easy_init();

	strCustomRequest = "GET " + strGetRequest;
	curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, strCustomRequest.c_str());

	curl_easy_setopt(pCurl, CURLOPT_HTTPGET, 1L);
	curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &simple);
	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, myCurlCallback);
	curl_easy_setopt(pCurl, CURLOPT_HEADERFUNCTION, writeHeader);
	curl_easy_setopt(pCurl, CURLOPT_WRITEHEADER, &headerMap);

	curl_easy_setopt(pCurl, CURLOPT_DEBUGFUNCTION, myDebugFunc);
	curl_easy_setopt(pCurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

	struct curl_slist *slist=NULL;
	str = "Host: " + bucket  + ".s3.amazonaws.com";
	slist = curl_slist_append(slist, str.c_str());
	str = "Date: " + strTimestamp;
	FILE_LOG(logDEBUG)<<"[Date]"<< str;
	slist = curl_slist_append(slist, str.c_str());
	str = "Authorization: AWS ";
	str += accessKey;
	str += ":";
	str += strSignature;
	FILE_LOG(logDEBUG)<<"[Authorization]"<< str;
	slist = curl_slist_append(slist, str.c_str());
	slist = curl_slist_append(slist, "Accept-Encoding: identity");
	slist = curl_slist_append(slist, "Transfer-Encoding: ");
	slist = curl_slist_append(slist, "User-Agent: Mozilla/4.0 (Compatible; ubuntu32; MyS3 1.0; Linux)");
	slist = curl_slist_append(slist, "Content-Type: text/html");

	if (slist != NULL)
	{
		curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, slist);
	}
	curl_easy_setopt(pCurl, CURLOPT_URL, strUrl.c_str());

	curl_easy_perform(pCurl);

	curl_easy_cleanup(pCurl);
	curl_slist_free_all(slist); /* free the list again */
	simplexml_deinitialize(&simple);

	for (int i = 0; i < stringPairVector.size(); i ++)
	{
		FILE_LOG(logDEBUG)<<"[key]" << stringPairVector[i].first <<"[UploadId]"<< stringPairVector[i].second;
	}
	return stringPairVector;
}


bool MyS3::initialize(const string& bucketName, const string& objectName, const string& fileName, size_t nSuggestedPartNumber,
	size_t nThreadNumber, const string& strMime)
{
	multiPartDesc.bucket = bucketName;
	multiPartDesc.object = objectName;
	multiPartDesc.file = fileName;
	FILE* stream = fopen(fileName.c_str(), "rb");
	if (stream != NULL)
	{
		fseek(stream, 0L, SEEK_END);
		long fileSize = ftell(stream);
		long partSize = fileSize / nSuggestedPartNumber;
		long lastPartSize = 0;
		// check part size if bigger than minimum of 5M requirement
		#define MinimumPartSize 5*1024*1024
		if ( partSize < MinimumPartSize)
		{
			nSuggestedPartNumber = fileSize / MinimumPartSize;
		}
		// at least 1
		if (nSuggestedPartNumber == 0)
		{
			nSuggestedPartNumber = 1;
		}
		partSize = fileSize / nSuggestedPartNumber;
		lastPartSize = fileSize % nSuggestedPartNumber;

		multiPartDesc.partCount = nSuggestedPartNumber;
		multiPartDesc.partDescVector.clear();
		long offset = 0;
		int i;
		PartDesc partDesc;
		for (i = 0; i < nSuggestedPartNumber; i ++)
		{
			partDesc.partId = i + 1;
			partDesc.offset = offset;
			partDesc.current = offset;
			if (i != nSuggestedPartNumber - 1)
			{
				partDesc.size = partSize;
			}
			else
			{
				partDesc.size = partSize + lastPartSize;
			}
			partDesc.stream = NULL;
			offset += partSize;
			partDesc.finished = false;
			//partDesc.md5 = calculateMD5(stream, partDesc.offset, partDesc.size);
			//partDesc.pMulti = &multiPartDesc;
			multiPartDesc.partDescVector.push_back(partDesc);
		}
		//addFileContentType(stream);
		multiPartDesc.contentType = strMime;
		multiPartDesc.accessKey = accessKey;
		multiPartDesc.secretAccessKey = secretAccessKey;
		multiPartDesc.nThreadNumber = nThreadNumber;
		/*
		int nThreadMax = sysconf(_SC_NPROCESSORS_CONF);
		if (nThreadMax != -1)
		{
			// should not be more than the core number???
			if (nThreadNumber > nThreadMax)
			{
				multiPartDesc.nThreadNumber = nThreadMax;
			}
		}
		*/
		if (multiPartDesc.nThreadNumber > multiPartDesc.partCount)
		{
			multiPartDesc.nThreadNumber = multiPartDesc.partCount;
		}
		pthread_mutex_init(&multiPartDesc.mutex, NULL);
		//fclose(stream);
		return true;
	}
	return false;
}


string doSignature(const string& strToSign, const string& strKey)
{
	MySH1 mySH1(strKey);
	mySH1.update(strToSign);
	string strSH1 = mySH1.finalize();
	MyBase64Encoder myBase64Encoder;
	myBase64Encoder.update(strSH1);
	return myBase64Encoder.finalize();
}

void MyS3::addDefaultHttpHeader(curl_slist*& slist, const string& strTimestamp, const string& strSignature)
{
	string str;
	str = "Host: " + multiPartDesc.bucket  + ".s3.amazonaws.com";
	slist = curl_slist_append(slist, str.c_str());

	str = "Date: " + strTimestamp;
	FILE_LOG(logDEBUG)<<"[Date]"<< str;
	slist = curl_slist_append(slist, str.c_str());

	str = "Authorization: AWS ";
	str += accessKey;
	str += ":";
	str += strSignature;
	FILE_LOG(logDEBUG)<<"[Authorization]"<< str;
	slist = curl_slist_append(slist, str.c_str());

	slist = curl_slist_append(slist, "Accept-Encoding: identity");

	slist = curl_slist_append(slist, "Transfer-Encoding: ");
	slist = curl_slist_append(slist, "User-Agent: Mozilla/4.0 (Compatible; ubuntu32; MyS3 1.0; Linux)");

}


string MyS3::createCompletePartBody()
{
	string str;
	str = "<CompleteMultipartUpload>";
	for (int i = 0; i < multiPartDesc.partCount; i ++)
	{
		str += "<Part><PartNumber>" + int2string(i+1) + "</PartNumber><ETag>";
		str += multiPartDesc.partDescVector[i].eTag;
		str += "</ETag></Part>";
	}
	str += "</CompleteMultipartUpload>";
	return str;
}

bool MyS3::initMultiPartUpload()
{
	string str;
	StringMap stringMap, headerMap;

	string strToSign, strPostRequest, strUrl, strSubUrl, strCustomRequest, strSignature;
	string strTimestamp = getTimestamp();

	strPostRequest = "/" + multiPartDesc.object + "?uploads";

	FILE_LOG(logDEBUG)<<"[strPostRequest]"<< strPostRequest;
	strSubUrl = "/" + multiPartDesc.bucket + strPostRequest;

	strUrl = "http://s3.amazonaws.com/" + multiPartDesc.bucket ;
	FILE_LOG(logDEBUG)<<"[strUrl]"<< strUrl;
	strToSign = "POST\n\n";
	strToSign += multiPartDesc.contentType;
	strToSign += "\n";
	strToSign += strTimestamp;
	strToSign += "\nx-amz-acl:public-read\nx-amz-storage-class:REDUCED_REDUNDANCY\n";
	strToSign += strSubUrl;

	FILE_LOG(logDEBUG)<<"[strToSign]"<< strToSign;

	doSignature(strToSign, string(secretAccessKey));

	strSignature = doSignature(strToSign, secretAccessKey);

	FILE_LOG(logDEBUG)<<"[strSignature]"<< strSignature;

	stringMap["UploadId"] = "";
	SimpleXml simple;
	simplexml_initialize(&simple, mySimpleXmlCallback, &stringMap);

	CURL* pCurl = curl_easy_init();

	strCustomRequest = "POST " + strPostRequest;
	curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, strCustomRequest.c_str());

	curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &simple);
	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, myCurlCallback);
	curl_easy_setopt(pCurl, CURLOPT_HEADERFUNCTION, writeHeader);
	curl_easy_setopt(pCurl, CURLOPT_WRITEHEADER, &headerMap);

	curl_easy_setopt(pCurl, CURLOPT_DEBUGFUNCTION, myDebugFunc);


	curl_easy_setopt(pCurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
	struct curl_slist *slist=NULL;
	addDefaultHttpHeader(slist, strTimestamp, strSignature);
	str = "Content-Type: " + multiPartDesc.contentType;
	slist = curl_slist_append(slist, str.c_str());

	slist = curl_slist_append(slist, "x-amz-acl: public-read");
	slist = curl_slist_append(slist, "x-amz-storage-class: REDUCED_REDUNDANCY");

	if (slist != NULL)
	{
		curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, slist);
	}
	curl_easy_setopt(pCurl, CURLOPT_URL, strUrl.c_str());

	curl_easy_perform(pCurl);

	curl_easy_cleanup(pCurl);
	curl_slist_free_all(slist); /* free the list again */
	simplexml_deinitialize(&simple);

	multiPartDesc.uploadId = stringMap["UploadId"];
	FILE_LOG(logDEBUG)<<"[UploadId]"<< multiPartDesc.uploadId;
	return true;
}

bool MyS3::uploadMultiPart()
{
	for (int i = 0; i < multiPartDesc.partCount; i ++)
	{
		if (!doUploadMultiPart(i))
		{
			return false;
		}
	}
	return true;
}

size_t myReadFunc(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	PartDesc* partDesc = (PartDesc*) userdata;
	//MultiPartDesc* pMulti = partDesc->pMulti;
	size_t toRead = size * nmemb;
	size_t leftToRead = partDesc->size - (partDesc->current - partDesc->offset);
	//pthread_mutex_lock(&pMulti->mutex);
	//fseek(partDesc->stream, partDesc->current, SEEK_SET);
	if (toRead > leftToRead)
	{
		toRead = leftToRead;
	}
	size_t actualRead = fread(ptr, 1, toRead, partDesc->stream);
	//pthread_mutex_unlock(&pMulti->mutex);
	partDesc->current+= actualRead;
	return actualRead;
}

bool MyS3::doUploadMultiPart(int index)
{
	string str;
	StringMap stringMap;
	string strToSign, strUrl, strSubUrl, strCustomRequest;
	string strPutRequest;
	StringMap headerMap;

	string strTimestamp = getTimestamp();
	//PUT /ObjectName?partNumber=PartNumber&uploadId=UploadId
	strPutRequest = "/" + multiPartDesc.object + "?partNumber=" + int2string(index + 1) + "&amp;uploadId=" + multiPartDesc.uploadId;

	FILE_LOG(logDEBUG)<<"[strPutRequest]"<< strPutRequest;

	strUrl = "http://s3.amazonaws.com/" + multiPartDesc.bucket ;
	FILE_LOG(logDEBUG)<<"[strUrl]"<< strUrl;
	strToSign = "PUT\n";
	strToSign += multiPartDesc.partDescVector[index].md5 + "\n";
	strToSign += multiPartDesc.contentType + "\n";
	strToSign += strTimestamp + "\n";
	strToSign += "/" + multiPartDesc.bucket + strPutRequest;

	FILE_LOG(logDEBUG)<<"[strToSign]"<< strToSign;

	string strSignature = doSignature(strToSign, secretAccessKey);

	FILE_LOG(logDEBUG)<<"[strSignature]"<< strSignature;
	SimpleXml simple;
	simplexml_initialize(&simple, mySimpleXmlCallback, &stringMap);

	CURL* pCurl = curl_easy_init();

	curl_easy_setopt(pCurl, CURLOPT_UPLOAD, 1L);

	curl_easy_setopt(pCurl, CURLOPT_READFUNCTION, myReadFunc);
	curl_easy_setopt(pCurl, CURLOPT_READDATA, &multiPartDesc.partDescVector[index]);

	strCustomRequest = "PUT " + strPutRequest;
	curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, strCustomRequest.c_str());
	FILE_LOG(logDEBUG)<<"[strPutRequest]"<< strPutRequest;

	curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &simple);
	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, myCurlCallback);
	curl_easy_setopt(pCurl, CURLOPT_HEADERFUNCTION, writeHeader);
	curl_easy_setopt(pCurl, CURLOPT_WRITEHEADER, &headerMap);

	curl_easy_setopt(pCurl, CURLOPT_DEBUGFUNCTION, myDebugFunc);

	curl_easy_setopt(pCurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
	struct curl_slist *slist=NULL;

	addDefaultHttpHeader(slist, strTimestamp, strSignature);

	str = "Content-MD5: " + multiPartDesc.partDescVector[index].md5;
	slist = curl_slist_append(slist, str.c_str());

	str = "Content-Length: " + int2string(multiPartDesc.partDescVector[index].size);
	slist = curl_slist_append(slist, str.c_str());

	str = "Content-Type: " + multiPartDesc.contentType;
	slist = curl_slist_append(slist, str.c_str());

	if (slist != NULL)
	{
		curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, slist);
	}
	curl_easy_setopt(pCurl, CURLOPT_URL, strUrl.c_str());

	curl_easy_perform(pCurl);

	curl_easy_cleanup(pCurl);
	curl_slist_free_all(slist); /* free the list again */
	simplexml_deinitialize(&simple);

	multiPartDesc.partDescVector[index].eTag = headerMap["ETag"];
	FILE_LOG(logDEBUG)<<"[ETag]"<< multiPartDesc.partDescVector[index].eTag;
	return true;
}

bool MyS3::completeMultiPartUpload()
{
	string str;
	StringMap stringMap;
	string strToSign, strUrl, strSubUrl, strCustomRequest;
	string strPostRequest, strBody;
	StringMap headerMap;

	strBody = createCompletePartBody();
	string strTimestamp = getTimestamp();

	strPostRequest = "/" + multiPartDesc.object + "?uploadId=" + multiPartDesc.uploadId;

	FILE_LOG(logDEBUG)<<"[strPostRequest]"<< strPostRequest;

	//strSubUrl = "/" + multiPartDesc.bucket + strPostRequest;
	strUrl = "http://s3.amazonaws.com/" + multiPartDesc.bucket ;
	FILE_LOG(logDEBUG)<<"[strUrl]"<< strUrl;
	strToSign = "POST\n\ntext/html\n";
	strToSign += strTimestamp + "\n";
	strToSign += "/" + multiPartDesc.bucket + strPostRequest;

	FILE_LOG(logDEBUG)<<"[strToSign]"<< strToSign;

	string strSignature = doSignature(strToSign, secretAccessKey);

	FILE_LOG(logDEBUG)<<"[strSignature]"<< strSignature;
	SimpleXml simple;
	simplexml_initialize(&simple, mySimpleXmlCallback, &stringMap);

	CURL* pCurl = curl_easy_init();

	curl_easy_setopt(pCurl, CURLOPT_POST, 1L);

	curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, strBody.c_str());
	curl_easy_setopt(pCurl, CURLOPT_POSTFIELDSIZE, strBody.size());

	strCustomRequest = "POST " + strPostRequest;
	curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, strCustomRequest.c_str());
	//curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, NULL);
	FILE_LOG(logDEBUG)<<"[strPostRequest]"<< strPostRequest;

	curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &simple);
	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, myCurlCallback);
	curl_easy_setopt(pCurl, CURLOPT_HEADERFUNCTION, writeHeader);
	curl_easy_setopt(pCurl, CURLOPT_WRITEHEADER, &headerMap);

	curl_easy_setopt(pCurl, CURLOPT_DEBUGFUNCTION, myDebugFunc);

	curl_easy_setopt(pCurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
	struct curl_slist *slist=NULL;

	addDefaultHttpHeader(slist, strTimestamp, strSignature);

	slist = curl_slist_append(slist, "Content-Type: text/html");

	str = "Content-Length: " + int2string(strBody.size());
	slist = curl_slist_append(slist, str.c_str());

	if (slist != NULL)
	{
		curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, slist);
	}
	curl_easy_setopt(pCurl, CURLOPT_URL, strUrl.c_str());

	curl_easy_perform(pCurl);

	curl_easy_cleanup(pCurl);
	curl_slist_free_all(slist); /* free the list again */
	simplexml_deinitialize(&simple);

	multiPartDesc.eTag = headerMap["ETag"];
	FILE_LOG(logDEBUG)<<"[ETag]"<< multiPartDesc.eTag;
	return true;
}

string getTimestamp()
{
	time_t after = time(NULL);
	char buffer[32];
	strftime(buffer, 32, "%a, %d %b %Y %H:%M:%S +0000", gmtime(&after));
	std::string in = buffer;
	//return urlEncode(in);
	return in;
}


int myDebugFunc(CURL * pCurl, curl_infotype infoType, char* ptrData , size_t nDataSize, void * userData)
{
	static FILE* streamDebug = NULL;
	if (streamDebug == NULL)
	{
		streamDebug = fopen("curlDebug.log", "w");
	}

	switch (infoType)
	{
	case CURLINFO_TEXT:
		fprintf(streamDebug, "[CURLINFO_TEXT]:\n");
		break;
	case CURLINFO_HEADER_IN:
		fprintf(streamDebug, "[CURLINFO_HEADER_IN]:\n");
		break;
	case CURLINFO_HEADER_OUT:
		fprintf(streamDebug, "[CURLINFO_HEADER_OUT]:\n");
		break;
	case CURLINFO_DATA_IN:
		fprintf(streamDebug, "[CURLINFO_DATA_IN]:\n");
		break;
	case CURLINFO_DATA_OUT:
		//fprintf(streamDebug, "[CURLINFO_DATA_OUT]: length=%d\n", nDataSize);
		break;
	}
	if (infoType != CURLINFO_DATA_OUT)
	{
		fwrite(ptrData, 1, nDataSize, streamDebug);
		fprintf(streamDebug, "\n");
	}

	fflush(streamDebug);
	return 0;
}

size_t writeHeader(void *ptr, size_t size, size_t nmemb, void *user)
{
	char header[32];
	char body[256];
	int len;
	StringMap* my= (StringMap*)user;
	//printf("size = %d, nmemb=%d\n", size, nmemb);
	char* str = (char*)ptr;
	char* sub = strstr(str, ": ");
	if (sub == NULL)
	{
		// actually the first line is NOT a header, i.e. "http 403 forbidden"
		len = size * nmemb - 2;
		if (len > 0 )
		{
			memcpy(body, ptr, len);
			body[len] = '\0';
			(*my)["status"] = body;
		}
	}
	else
	{
		len = sub - str;
		memcpy(header, str, len);
		header[len] = '\0';
		// remove the trailing \r\n
		len = size * nmemb - len - 2 - 2;
		memcpy(body, sub + 2, len);
		body[len] = '\0';
		(*my)[header] = body;
	}
	FILE_LOG(logDEBUG)<<"[writeHeader]"<<str;
	return size * nmemb;
}

size_t myCurlCallback(void *ptr, size_t size, size_t nmemb, void *user)
{
	SimpleXml*simple = (SimpleXml*)user;
	char* data = (char*) ptr;
	size_t result = size*nmemb;
	FILE_LOG(logDEBUG)<<"[myCurlCallback]"<<data;
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
	StringMap* myMapPtr = (StringMap*) callbackData;
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
		std::string myKey = sub;
		StringMap::iterator it = myMapPtr->find(std::string(sub));
		if (it != myMapPtr->end())
		{
			(*myMapPtr)[it->first] = buffer;
			FILE_LOG(logDEBUG)<<"[mySimpleXmlCallback]sub="<<sub<<";buffer="<<buffer;
		}
	}
	return 0;
}


int mySimpleXmlGetUploadIdCallback(const char *elementPath, const char *data, int dataLen, void *callbackData)
{
	static string currentKey;
	char buffer[1024];
	StringPairVector* myStringPairVectorPtr = (StringPairVector*) callbackData;
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
		std::string myKey = sub;
		if (myKey.compare("Key") == 0)
		{
			currentKey = buffer;
		}
		else
		{
			if (myKey.compare("UploadId") == 0)
			{
				myStringPairVectorPtr->push_back(StringStringPair(currentKey, string(buffer)));
				FILE_LOG(logDEBUG)<<"[mySimpleXmlGetUploadIdCallback]key="<<currentKey<<";uploadId="<<buffer;
			}
		}
	}
	return 0;
}

