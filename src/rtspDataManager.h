/*
 * @Author: xtt
 * @Date: 2021-01-08 18:32:32
 * @Description: ...
 * @LastEditTime: 2021-03-09 15:46:09
 */

#include "dataModel.h"
#include "socketbase.h"
#include <atomic>
namespace std
{
        class thread;
}

class RtspDataManager
{
private:
        RtspDataManager();
        ~RtspDataManager();

public:
        static RtspDataManager *getInstance();
        void login(const char *_dstIp, unsigned short _dstPort, CODEC_TYPE &_type);
        inline void setSSRC(uint32_t _SSRC)
        {
                if (0 == SSRC) SSRC = _SSRC;
        }
        inline void setRtpVersion(unsigned char _version)
        {
                if (0 == VERSION) VERSION = _version;
        }
        inline void setSeq(uint16_t seq)
        {
                static uint16_t max = 0;
                SEQ = seq + (65536 * wrap_around_count);
                if (seq == 65535)
                {
                        wrap_around_count++;
                        max = 0;
                }
                max = std::max<uint16_t>(SEQ, max);
                SEQ = max;
        }

        void play();
        void stop();

private:
        static void receive_tcp_thread(void *);
        static void send_report_thread(void *);
        static void recv_report_thread(void *);
        void parser(const char *, int);
        void calMd5();
        char *calResponse(const char *cmd);
        void start();

private:
        int load_type;
        char *psend_buffer = nullptr;
        CODEC_TYPE TYPE;
        std::thread *pThread_tcp = nullptr;
        std::thread *pThread_report_send = nullptr;
        std::thread *pThread_report_recv = nullptr;
        SocketTcp *pSocketTcp = nullptr;
        SocketUdp *pSocketUdp = nullptr;
        uint32_t SSRC = 0;
        std::atomic_uint16_t SEQ;
        uint16_t wrap_around_count = 1;
        uint32_t LSR;
        uint64_t SR_Time = 0;
        unsigned char VERSION = 0;
};