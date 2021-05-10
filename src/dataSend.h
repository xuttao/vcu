/*
 * @Author: xtt
 * @Date: 2021-01-18 12:54:37
 * @Description: ...
 * @LastEditTime: 2021-04-07 12:07:46
 */
#include "common.h"
#include "receiveModel.h"
#include "socketbase.h"
#include <atomic>
#include <functional>
class BoxInfo2;

namespace std
{
        class thread;
}

class DataSend
{
private:
        DataSend();
        ~DataSend();

private:
        SocketUdp *pSocket = nullptr;
        SocketTcp *pSocketTcp = nullptr;
        std::thread *pThread = nullptr;
        std::atomic_bool is_stop = {true};
        std::atomic_bool can_send_box = {false};
        std::atomic_bool can_send_con = {false};
        std::atomic_uint32_t pic_index = {0};
        std::atomic_uint32_t times = {0};

public:
        static DataSend *getInstance();
        void sendBox(BoxInfo2 *pBoxinfo, int num, uint32_t id, uint64_t _time);
        inline bool canSendBox() const { return can_send_box; }
        inline bool canSendCon() const { return can_send_con; }
        inline uint32_t get_index() { return ++pic_index; }
        inline uint32_t get_times() { return times; }
        inline void set_streamInfo(uint8_t _type)
        {
                if (-1 == type) type = _type;
        }
        void setpf(std::function<void()> _pf) { pf = _pf; }

private:
        bool is_send = false;
        std::function<void()> pf;
        std::atomic_int8_t type = {-1};
        void send_streamInfo();

private:
        static void receive_thread(void *);
};