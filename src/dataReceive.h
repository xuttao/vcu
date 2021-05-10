/*
 * @Author: xtt
 * @Date: 2020-12-01 15:18:38
 * @Description: ...
 * @LastEditTime: 2020-12-24 13:31:41
 */
#include "receiveModel.h"
#include "socketbase.h"
#include "yoloDetect.h"
#include <atomic>
#include <thread>

class DataReceive
{
public:
        DataReceive();
        ~DataReceive();

public:
        void init(const std::string &_ip, int _port);
        void start();
        void stop();
        void send_res(BoxInfo2 &boxinfo, unsigned char *data, int len);
        void send_res2(BoxInfo2 *pBoxinfo, int num, uint32_t id);

private:
        std::thread *pThread_receive = nullptr;
        std::thread *pThread_parser = nullptr;
        std::atomic_bool isStop;
        SocketUdp *pSocket = nullptr;

private:
        static void *receive_thread(void *);
        static void *parser_thread(void *);
};