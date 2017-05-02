#pragma once

#include <string>
#include <vector>
#ifdef _WINDOWS_
#include <Windows.h> 
#else
//#include "XTCrossPlatform.h"
typedef unsigned char UINT8;
#endif
std::string base64_encode(unsigned char const* , unsigned int len);
std::string base64_decode(std::string const& s);
void base64_decodebytes(std::string const& encoded_string, std::vector<UINT8> &data);
