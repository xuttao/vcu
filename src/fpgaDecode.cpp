#include "fpgaDecode.h"

#include <arpa/inet.h>

#include <atomic>
#include <memory>

#include "log.h"
#define START 0xFF      //
#define QUANTIFY 0xDB   //量化表
#define IMGINFO 0xC0    //图片信息
#define TABLE 0xC4      //ac dc 表的起始
#define DATA_START 0xDA //数据开始
#define DATA_END 0xD9   //数据结束
#define DATA_MOD 0x10

#define HUFF_LOOKAHEAD 8
#define NUM_HUFF_TBLS 4
#define D_MAX_BLOCKS_IN_MCU 10
#define DCTSIZE2 64

namespace
{
        const unsigned int QtAddr = 0x3C000100;
        const unsigned int HuffAddr = 0x3C000200;
        const unsigned int DataReadAddr = 0x3C001500;
        const unsigned int DataWriteAddr = 0x3C0F1500;
        const unsigned int DataSize = 0xF000;
        const unsigned int dataNumBlocks_Qt = 0x08;
        const unsigned char dataBlockSize_Qt = 0x01;
        const unsigned int dataNumBlocks_Huff = 0x23;
        const unsigned char dataBlockSize_Huff = 0x01;
        const unsigned char dataBlockSize_data = 0x01;
        const unsigned int end = 0xFFFFFFFF;

        // const unsigned int DataWriteAddr2[] = {0x3c0F1500,
        //                                        0x3C169500,
        //                                        0x3C1E1500,
        //                                        0x3C259500,
        //                                        0x3C2D1500,
        //                                        0x3C349500,
        //                                        0x3C3C1500,
        //                                        0x3C439500};
        // std::atomic<int> pic_num = {0};
        // unsigned char* pHead = NULL;
        // unsigned char* pQt = NULL;
        // unsigned char* pHuff = NULL;
        //unsigned char* pDetData = NULL;
} // namespace

void initDecode()
{
        // pHead = (unsigned char*)malloc(LEN_HEAD);
        // pQt = (unsigned char*)malloc(LEN_QT);
        // pHuff = (unsigned char*)malloc(LEN_HUFF);
}

inline int getDataLength(const unsigned char *pData)
{
        return (pData[0]) * 0x100 + pData[1];
}

int getFileEnd(unsigned char *data)
{
        unsigned char *pData = data;
        int length = 0;
        while (true)
        {
                length++;
                if (*pData++ == 0xFF)
                {
                        length++;
                        if (*pData++ == DATA_END)
                        {
                                break;
                        }
                }
        }
        return length;
}

int getFileEnd2(unsigned char *data, int pos, int end_pos)
{
        unsigned char *pData = data;
        int length = 0;
        pData += end_pos - pos;
        for (;;)
        {
                if (*pData-- == DATA_END)
                {
                        if (*pData == 0xFF)
                        {
                                break;
                        }
                }
                length++;
        }
        return end_pos - length - pos + 1;
        // while (true)
        // {
        //         length++;
        //         if (*pData++ == 0xFF)
        //         {
        //                 length++;
        //                 if (*pData++ == DATA_END)
        //                 {
        //                         break;
        //                 }
        //         }
        // }

        //  return length;
}

int getDecodeTable(unsigned char *_pHead, unsigned char *_pQt, unsigned char *_pHuff, unsigned char *&pDetData, int &Detlen, const char *_pFilePath)
{
        FILE *fp = fopen(_pFilePath, "rb");
        fseek(fp, 0L, SEEK_END);
        int length = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        char *picData = (char *)malloc(length);
        fread(picData, length, 1, fp);
        fclose(fp);

        unsigned char *pHead = _pHead;
        unsigned char *pQt = _pQt;
        unsigned char *pHuff = _pHuff;
        memset(pHead, 0, 96);
        //unsigned char* pDetData = _pData;
        int posTot = 0;
        int posQt = 0;
        int posHead = 0;
        int posHuff = 0;
        unsigned short huffLength = 0;
        int mod = 0;
        unsigned char *pFileData = (unsigned char *)picData;
        if (pFileData[2] != 0xFF || pFileData[3] != 0xE0)
        {
                return -1;
        }
        // unsigned char* pDetData = NULL;
        // if (pDetData)
        // {
        //         delete[] pDetData;
        // }
        memcpy(pHead, &QtAddr, 4);
        posHead += 4;
        memcpy(pHead + posHead, &dataNumBlocks_Qt, 3);
        posHead += 3;
        memcpy(pHead + posHead, &dataBlockSize_Qt, 1);
        posHead += 1;
        memcpy(pHead + posHead, &HuffAddr, 4);
        posHead += 4;
        memcpy(pHead + posHead, &dataNumBlocks_Huff, 3);
        posHead += 3;
        memcpy(pHead + posHead, &dataBlockSize_Huff, 1);
        posHead += 1;
        memcpy(pHead + posHead, &DataReadAddr, 4);
        posHead += 4;
        //data num blocks 3个字节空挡
        //memcpy(pHead + posHead, )
        int posDataBlocks = posHead;
        posHead += 3;
        memcpy(pHead + posHead, &dataBlockSize_data, 1);
        posHead += 1;
        //memcpy(pHead + posHead, &DataWriteAddr, 4);
        // unsigned int ss = pic_num;
        // pic_num++;
        // memcpy(pHead + posHead, &DataWriteAddr2[ss % 8], 4);
        posHead += 4;
        memcpy(pHead + posHead, &DataSize, 3);
        posHead += 3;
        memcpy(pHead + posHead, &dataBlockSize_data, 1);
        posHead += 1;
        memcpy(pHead + 92, &end, 4);

        bool isEnd = false;
        for (int i = 0; i < length && !isEnd; i++)
        {
                unsigned char dataKey = *pFileData++;
                posTot++;
                if (dataKey == 0xFF)
                {
                        int len = 0;
                        dataKey = *pFileData++;
                        posTot++;
                        switch (dataKey)
                        {
                                case QUANTIFY:
                                {
                                        len = getDataLength(pFileData);
                                        pFileData += 3;
                                        posTot += 3;
                                        len -= 3;
                                        memcpy(pQt + posQt, pFileData, len);
                                        posQt += len;
                                }
                                break;
                                case IMGINFO:
                                {
                                        len = getDataLength(pFileData);
                                        unsigned char *iData = pFileData;
                                        iData += 3;
                                        unsigned short height = ntohs(*(unsigned short *)(iData));
                                        unsigned short width = ntohs(*(unsigned short *)(iData + 2));
                                        if (height != 512 || width != 640)
                                        {
                                                return -1;
                                        }
                                        memcpy(pHead + posHead, &height, 2);
                                        posHead += 2;
                                        memcpy(pHead + posHead, &width, 2); //
                                        posHead += 2;
                                        iData += 4;
                                        int count = *iData;
                                        for (int i = 0; i < count; i++)
                                        {
                                                iData += 3;
                                                memcpy(pHead + posHead + i, iData, 1);
                                        }
                                        posHead += count;
                                }
                                break;
                                case TABLE:
                                {
                                        unsigned short value = 0;
                                        len = getDataLength(pFileData);
                                        memcpy(pHuff + posHuff, pFileData, len);
                                        posHuff += len;
                                        pFileData += 3;
                                        posTot += 3;
                                        unsigned short pdata[16] = {0};

                                        //计算huff 表1-16 一一对应地址起始值
                                        for (int8_t i = 1; i <= 16; i++)
                                        {
                                                if (*pFileData != 0)
                                                {
                                                        pdata[i - 1] = ntohs(value);
                                                        int8_t count = *pFileData;
                                                        for (int8_t j = 0; j < count; j++)
                                                                value++;
                                                }
                                                value = value << 1;
                                                pFileData++;
                                                posTot++;
                                        }

                                        memcpy(pHuff + posHuff, pdata, 32);
                                        posHuff += 32;
                                        huffLength += len;
                                        huffLength += 32;
                                }
                                break;
                                case DATA_START:
                                {
                                        //保存图像信息中huff表查找id
                                        unsigned char *iData = pFileData;
                                        iData += 2; //
                                        int count = *iData;
                                        int posData = 0;
                                        for (int i = 0; i < count; i++)
                                        {
                                                iData += 2;
                                                memcpy(pHead + posHead + i, iData, 1);
                                        }
                                        posHead += count;

                                        len = getDataLength(pFileData);
                                        pFileData += len;
                                        posTot += len;
                                        const unsigned char cMa = 0xFF;
                                        //记录huff表总长度，以十六字节补1
                                        if (huffLength > 0)
                                        {
                                                mod = huffLength % DATA_MOD;
                                                short num = 0;
                                                num = DATA_MOD - mod;
                                                huffLength += num;

                                                // memset(pHuff + posHuff, cMa, num);
                                                // posHuff += num;
                                                while (num--)
                                                {
                                                        memcpy(pHuff + posHuff, &cMa, 1);
                                                        posHuff += 1;
                                                }
                                                int huffTemp = htons(huffLength);
                                                if (huffLength != 560)
                                                {
                                                        LOG_WARN("huff length!=560");
                                                }
                                                // memcpy(pHead + posHead, &huffTemp, sizeof(unsigned short));
                                                // posHead += sizeof(unsigned short);
                                        }
                                        //len = getFileEnd(pFileData);
                                        len = getFileEnd2(pFileData, posTot, length - 1);
                                        mod = len & 15;
                                        if (mod != 0)
                                        {
                                                int leftnum = DATA_MOD - mod;
                                                int fileLen = len;
                                                len += leftnum;
                                                pDetData = new unsigned char[len];
                                                memcpy(pDetData, pFileData, fileLen);
                                                memset(pDetData + fileLen, cMa, leftnum);
                                        }
                                        else
                                        {
                                                pDetData = new unsigned char[len];
                                                memcpy(pDetData, pFileData, len);
                                        }
                                        int lenTemp = len / 16;
                                        memcpy(pHead + posDataBlocks, &lenTemp, 3);
                                        //posHead += 4;
                                        Detlen = len;
                                        isEnd = true;
                                }
                                break;
                                default:
                                        break;
                        }
                }
        }
        return 0;
}
