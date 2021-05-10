#pragma once
#include <unistd.h>

#include <iostream>
#include <mutex>
#include <thread>
class InputBufferManager
{
public:
        InputBufferManager();
        InputBufferManager(int _size, int len);
        ~InputBufferManager();

public:
        inline unsigned char *getBufferToSavePicMt();
        inline unsigned char *getBufferToSavePicMtTest();
        inline void releaseBufferMt();
        inline void reInit();

private:
        void init(int);

private:
        std::mutex buffer_mutex;
        uint64_t buffer_index_produce = 0;
        uint64_t buffer_index_consume = 0;
        unsigned char *pBuffer = nullptr;
        size_t bufferLen = 640 * 512 * 3; //单张图片字节
        size_t bufferNum = 32;            //图片个数,必须是2的幂
        int n = 0;
};

InputBufferManager::InputBufferManager()
{
        init(bufferNum);
}

InputBufferManager::InputBufferManager(int _size, int len)
{
        bufferLen = len;
        init(_size);
}

InputBufferManager::~InputBufferManager()
{
        delete[] pBuffer;
}

void InputBufferManager::init(int _size)
{
        bufferNum = _size;
        pBuffer = new unsigned char[bufferLen * bufferNum];
        n = bufferNum - 1;
}

inline void InputBufferManager::reInit()
{
        buffer_index_produce = 0;
        buffer_index_consume = 0;
}

inline unsigned char *InputBufferManager::getBufferToSavePicMt()
{
        for (;;)
        {
                buffer_mutex.lock();
                if (buffer_index_produce - buffer_index_consume > n - 1)
                {
                        buffer_mutex.unlock();
                        usleep(5 * 1000);
                        continue;
                }
                unsigned char *pRes = pBuffer + (buffer_index_produce & n) * bufferLen;
                buffer_index_produce++;

                buffer_mutex.unlock();

                return pRes;
        }
}

inline unsigned char *InputBufferManager::getBufferToSavePicMtTest()
{
        for (;;)
        {
                buffer_mutex.lock();
                // if(buffer_index_produce-buffer_index_consume>n-1){
                //     buffer_mutex.unlock();
                //     usleep(2*1000);
                //     continue;
                // }
                unsigned char *pRes = pBuffer + (buffer_index_produce % bufferNum) * bufferLen;
                buffer_index_produce++;

                buffer_mutex.unlock();

                return pRes;
        }
}

inline void InputBufferManager::releaseBufferMt()
{
        buffer_mutex.lock();

        buffer_index_consume++;

        buffer_mutex.unlock();
}
