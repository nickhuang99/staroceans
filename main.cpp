/*
 * main.cpp
 *
 *  Created on: Oct 6, 2013
 *      Author: nick
 */
#include <cstdio>

#include "input.h"
#include "MyLibx264.h"

using namespace std;

int main()
{
	const char* inputFileName = "/home/nick/MyProjects/MyCapture/output.yuv";
	const char* outputFileName = "./output.264";
	const uint32_t width = 640, height = 480;
	uint8_t buffer[width*height*2];
	MyInput input;
	MyLibx264 x264;
	if (!input.init(inputFileName, 640, 480))
	{
		printf("init input failed!\n");
		return -1;
	}
	if (!x264.init(outputFileName, 640, 480))
	{
		printf("init x264 failed!\n");
		return -1;
	}

	while (input.read_frame(buffer))
	{
		if (!x264.encode_frame(buffer))
		{
			printf("encode frame failed!\n");
			break;
		}
	}
	x264.uninit();
	input.uninit();
	return 0;
}
