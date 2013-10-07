/*
 * input.cpp
 *
 *  Created on: Oct 6, 2013
 *      Author: nick
 */

#include "input.h"

bool MyInput::init(const char* fileName, unsigned int width, unsigned int height, uint32_t chromeFormat)
{
	m_width = width;
	m_height = height;
	input = fopen(fileName, "r+b");
	if (input == NULL)
	{
		return false;
	}
	m_chrome_format = chromeFormat;
	return true;
}

bool MyInput::read_frame(uint8_t* src_ptr)
{
	uint32_t r, c;
	uint8_t* ptr = src_ptr;
	// y
	int result = fread(ptr, 1, m_width*m_height, input);
	if (result != m_width*m_height)
	{
		return false;
	}
	ptr += m_width*m_height;
	switch (m_chrome_format)
	{
	case YUV420P:
		fread(ptr, 1, m_width*m_height/2, input);
		break;
	case YUV422P:
		for ( r = 0; r < m_height; r ++)
		{
			for (c = 0; c < m_width; c ++)
			{
				// let's do it later
			}
		}
		break;
	}
	return true;
}


void MyInput::uninit()
{
	fclose(input);
}
