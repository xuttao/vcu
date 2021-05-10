#include "dataReceive.h"
#include "dataControl.h"
#include "log.h"
#include <QSemaphore>
#include <mutex>
#include <pthread.h>
namespace
{
        ByteArray arr;
        ByteArray pic;
        int lastPacketSize;
        std::mutex recv_mutex;
        QSemaphore recv_sem;
        QSemaphore send_sem;

        pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
        pthread_cond_t conwait = PTHREAD_COND_INITIALIZER;
        pthread_rwlock_t mutex_stop = PTHREAD_RWLOCK_INITIALIZER;
        bool recv_status = false;
} // namespace

DataReceive::DataReceive()
{
}

DataReceive::~DataReceive()
{
        isStop = true;
        recv_sem.release();
        pThread_receive->join();
        pThread_parser->join();
        delete pThread_receive;
        delete pThread_parser;
}

void DataReceive::init(const std::string &_ip, int _port)
{
        pSocket = new SocketUdp;
        bool res = pSocket->createUdpServer(_ip, _port);

        FSERVO_CHECK(res);
}

void DataReceive::start()
{
        isStop = false;
        pThread_receive = new std::thread(receive_thread, (void *)this);
        pThread_parser = new std::thread(parser_thread, (void *)this);
}

void DataReceive::stop()
{
        isStop = true;
        recv_sem.release();
}

void DataReceive::send_res(BoxInfo2 &boxinfo, unsigned char *data, int len)
{
        int size = sizeof(boxinfo);
        int signle_packet_len = PACKET_LEN + size;

        int packet_num;
        if (len % PACKET_LEN == 0)
        {
                packet_num = len / PACKET_LEN;
        }
        else
        {
                packet_num = len / PACKET_LEN + 1;
        }

        int last_packet_len = size + len - (packet_num - 1) * PACKET_LEN;
        int last_packet_con_len = len - (packet_num - 1) * PACKET_LEN;

        int packet_size1 = PACKET_HEAD_LEN + PACKET_LEN + size + PACKET_END_LEN;
        int packet_size2 = PACKET_HEAD_LEN + last_packet_len + PACKET_END_LEN;

        char *pPacket1 = new char[packet_size1];
        char *pPacket2 = new char[packet_size2];

        PacketHead head(PACKET_HEAD_FLAG, BOX, 0, packet_num, 0);
        PacketEnd end;

        for (int i = 0; i < packet_num; i++)
        {
                head.curFrame = i + 1;
                if (i == packet_num - 1)
                {
                        head.frameSize = last_packet_len;

                        memcpy(pPacket2, &head, PACKET_HEAD_LEN);
                        memcpy(pPacket2 + PACKET_HEAD_LEN, &boxinfo, size);
                        memcpy(pPacket2 + PACKET_HEAD_LEN + size, data + len - last_packet_con_len, last_packet_con_len);
                        memcpy(pPacket2 + PACKET_HEAD_LEN + last_packet_len, &end, PACKET_END_LEN);

                        bool ret = pSocket->send(pPacket2, packet_size2);
                        FSERVO_CHECK(ret);
                }
                else
                {
                        head.frameSize = PACKET_LEN + size;

                        memcpy(pPacket1, &head, PACKET_HEAD_LEN);
                        memcpy(pPacket1 + PACKET_HEAD_LEN, &boxinfo, size);
                        memcpy(pPacket1 + PACKET_HEAD_LEN + size, data + (i * PACKET_LEN), PACKET_LEN);
                        memcpy(pPacket1 + PACKET_HEAD_LEN + size + PACKET_LEN, &end, PACKET_END_LEN);

                        bool ret = pSocket->send(pPacket1, packet_size1);
                        FSERVO_CHECK(ret);
                }

                send_sem.acquire();
        }

        delete[] pPacket1;
        delete[] pPacket2;
}

#define PACKET_BOX_LEN (42 * sizeof(BoxInfo2))
void DataReceive::send_res2(BoxInfo2 *pBoxinfo, int num, uint32_t id)
{
        int size = sizeof(BoxInfo2);
        int len = num * size;

        //42个框打一个包

        int packet_num;
        if (len % PACKET_BOX_LEN == 0)
        {
                packet_num = len / PACKET_BOX_LEN;
        }
        else
        {
                packet_num = len / PACKET_BOX_LEN + 1;
        }

        int last_packet_len = len - (packet_num - 1) * PACKET_BOX_LEN;

        int packet_size1 = PACKET_HEAD_LEN + PACKET_BOX_LEN + PACKET_END_LEN;
        int packet_size2 = PACKET_HEAD_LEN + last_packet_len + PACKET_END_LEN;

        char *pPacket1 = new char[packet_size1];
        char *pPacket2 = new char[packet_size2];

        PacketHead head(PACKET_HEAD_FLAG, BOX, 0, packet_num, 0);
        PacketEnd end;

        head.picID = id;

        char *pSrc = (char *)pBoxinfo;

        for (int i = 0; i < packet_num; i++)
        {
                head.curFrame = i + 1;
                if (i == packet_num - 1)
                {
                        head.frameSize = last_packet_len;

                        memcpy(pPacket2, &head, PACKET_HEAD_LEN);
                        memcpy(pPacket2 + PACKET_HEAD_LEN, pSrc + len - last_packet_len, last_packet_len);
                        memcpy(pPacket2 + PACKET_HEAD_LEN + last_packet_len, &end, PACKET_END_LEN);

                        bool ret = pSocket->send(pPacket2, packet_size2);
                        FSERVO_CHECK(ret);
                }
                else
                {
                        head.frameSize = PACKET_BOX_LEN;

                        memcpy(pPacket1, &head, PACKET_HEAD_LEN);
                        memcpy(pPacket1 + PACKET_HEAD_LEN, pSrc + (i * PACKET_BOX_LEN), PACKET_BOX_LEN);
                        memcpy(pPacket1 + PACKET_HEAD_LEN + PACKET_BOX_LEN, &end, PACKET_END_LEN);

                        bool ret = pSocket->send(pPacket1, packet_size1);
                        FSERVO_CHECK(ret);
                }

                send_sem.acquire();
        }

        delete[] pPacket1;
        delete[] pPacket2;
}

void *DataReceive::parser_thread(void *arg)
{
        LOG_INFO("parser thread start ! id:%u", std::this_thread::get_id());

        DataReceive *pThis = static_cast<DataReceive *>(arg);

        for (;;)
        {
                if (pThis->isStop) return nullptr;

                recv_sem.acquire();

                if (pThis->isStop) return nullptr;

                recv_mutex.lock();
                ByteArray lastData = arr.mid(arr.size() - lastPacketSize, lastPacketSize);
                recv_mutex.unlock();

                PacketHead *pHead = (PacketHead *)lastData.data();

                if (pHead->curFrame == 1)
                {
                        pic.clear();
                        recv_mutex.lock();
                        arr.clear();
                        recv_mutex.unlock();
                }

                pic.append(lastData.data() + PACKET_HEAD_LEN, pHead->frameSize);

                if (pHead->curFrame == pHead->totFrame)
                {
                        char *ptemp = new char[pic.size()];
                        memcpy(ptemp, pic.data(), pic.size());

                        DataControl::setRgbData(ptemp, pic.size(), pHead->picID);

                        recv_mutex.lock();
                        arr.clear();
                        recv_mutex.unlock();

                        //LOG_INFO("receive rgb data,size:%d", pic.size());
                        pic.clear();
                }

                pthread_mutex_lock(&mutex);
                recv_status = true;
                pthread_cond_signal(&conwait);
                pthread_mutex_unlock(&mutex);
        }
        LOG_INFO("parser thread exit");
}

void *DataReceive::receive_thread(void *arg)
{
        LOG_INFO("receive thread start ! id:%u", std::this_thread::get_id());

        DataReceive *pThis = static_cast<DataReceive *>(arg);
        int recvsize = PACKET_HEAD_LEN + PACKET_LEN + PACKET_END_LEN;
        char *recvbuffer = new char[recvsize];

        PacketHead head(PACKET_HEAD_FLAG, RES, 1, 1, 0);
        PacketEnd end;

        int sendsize = PACKET_HEAD_LEN + PACKET_END_LEN;
        char *sendbuffer = new char[sendsize];
        memcpy(sendbuffer, &head, PACKET_HEAD_LEN);
        memcpy(sendbuffer + PACKET_HEAD_LEN, &end, PACKET_END_LEN);

        for (;;)
        {
                if (pThis->isStop) return nullptr;

                int ret = pThis->pSocket->read(recvbuffer, recvsize);
                if (ret == -1)
                {
                        usleep(10);
                        continue;
                }

                PacketHead *pHead = (PacketHead *)recvbuffer;
                if (pHead->frameType == RES)
                {
                        send_sem.release();
                        continue;
                }

                recv_mutex.lock();

                arr.append(recvbuffer, ret);
                lastPacketSize = ret;

                recv_mutex.unlock();

                recv_sem.release();

                pthread_mutex_lock(&mutex);
                while (!recv_status)
                {
                        pthread_cond_wait(&conwait, &mutex);
                }
                recv_status = false;
                pthread_mutex_unlock(&mutex);

                pThis->pSocket->send(sendbuffer, sendsize);
        }

        delete[] recvbuffer;
        delete[] sendbuffer;

        LOG_INFO("receive thread exit");
}