/*
 * input.h
 *
 *  Created on: Oct 6, 2013
 *      Author: nick
 */

#ifndef INPUT_H_
#define INPUT_H_
#include <cstdio>
#include <stdint.h>

using namespace std;

const uint32_t YUV420P = 0;
const uint32_t YUV422P = 1;

class MyInput
{
private:
	uint32_t m_width, m_height;
	FILE* input;
	uint32_t m_chrome_format;
public:
	bool init(const char* fileName, unsigned int width, unsigned int height, uint32_t chromeFormat=YUV420P);
	bool read_frame(uint8_t* ptr);
	void uninit();
};


#endif /* INPUT_H_ */
