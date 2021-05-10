#include "dataControl.h"
#include "dataModel.hpp"
#include "log.h"
#include <mutex>
#include <queue>
#include <unistd.h>
namespace
{
        std::mutex queue_mutex;
        std::queue<RgbData *> rgb_queue;
} // namespace

void DataControl::setRgbData(void *pdata, int size, uint32_t id)
{
        queue_mutex.lock();
        if (rgb_queue.size() > 10)
        {
                auto *p = rgb_queue.front();
                rgb_queue.pop();
                delete[] p->pRgb;
                delete p;
                // queue_mutex.unlock();
                // delete[] pdata;
                // return;
        }
        RgbData *ptemp = new RgbData;
        ptemp->pRgb = (char *)pdata;
        ptemp->rgbSize = size;
        ptemp->picID = id;

        rgb_queue.push(ptemp);

        queue_mutex.unlock();
}

void DataControl::clearRgbData()
{
        queue_mutex.lock();
        while (!rgb_queue.empty())
        {
                auto *p = rgb_queue.front();
                rgb_queue.pop();
                delete[] p->pRgb;
                delete p;
        }
        queue_mutex.unlock();
}

void *DataControl::getRgbData()
{
        int times = 0;
        for (;;)
        {
                queue_mutex.lock();
                if (rgb_queue.empty())
                {
                        queue_mutex.unlock();
#ifdef _Unix
                        usleep(10000);
#else
            std::this_thread::sleep_for(chrono::microseconds(3000);
#endif
                        // times++;
                        // if (times > 10000)
                        // {
                        //         LOG_ERR("no receive !!!");
                        //         exit(-1);
                        // }
                        continue;
                }
                RgbData *pRes = rgb_queue.front();
                rgb_queue.pop();
                queue_mutex.unlock();

                return pRes;
        }
}