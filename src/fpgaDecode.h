#pragma once

#include <stdio.h>

#define LEN_QT 128
#define LEN_HUFF 560
#define LEN_HEAD 96

void initDecode();

int getDecodeTable(unsigned char* _pHead, unsigned char* _pQt, unsigned char* _pHuff, unsigned char*& _pData, int& len, const char* _pFilePath);