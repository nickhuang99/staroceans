#include <iostream>
#include "stdafx.h"
#include "myS3.h"
#include <time.h>
#include <stdlib.h>


using namespace std;
using namespace MyS3MultiPartUploader;

int main(int argc, char** argv)
{
	size_t start = time(NULL);
	MyS3 myS3;
	if (argc == 7)
	{
		string bucket, object, file, strMime;
		int partNumber, threadNumber;

		bucket = argv[1];
		object = argv[2];
		file = argv[3];
		partNumber = atoi(argv[4]);
		threadNumber = atoi(argv[5]);
		strMime = argv[6];
		//myS3.multiPartUpload(string("nickhuang99"), string("japanese.rmvb"), string("japanese.rmvb"), 4, 6);
		myS3.multiPartUpload(bucket, object, file, partNumber, threadNumber, strMime);
		start = time(NULL) - start;
		printf("total time: %d\n", start);
	}
	else
	{
		if (argc == 2)
		{
			string bucket = argv[1];
			StringPairVector stringPairVector = myS3.getAllUploadId(bucket);
			for (int i = 0; i < stringPairVector.size(); i ++)
			{
				printf("[key]%s : [uploadId] %s\n", stringPairVector[i].first.c_str(), stringPairVector[i].second.c_str());
				if (myS3.abortUpload(bucket, stringPairVector[i].first, stringPairVector[i].second))
				{
					printf("success\n");
				}
			}
		}
		else
		{
			printf("usage: %s bucket object file partNumber threadNumber mimeType\n", argv[0]);
		}
	}
	/*
	MyS3 myS3;
	string bucket = "nickhuang99";
	StringPairVector stringPairVector = myS3.getAllUploadId(bucket);
	for (int i = 0; i < stringPairVector.size(); i ++)
	{
		printf("[key]%s : [uploadId] %s\n", stringPairVector[i].first.c_str(), stringPairVector[i].second.c_str());
		if (myS3.abortUpload(bucket, stringPairVector[i].first, stringPairVector[i].second))
		{
			printf("success\n");
		}
	}
	*/
    return 0;
}
