#include "rtspDataManager.h"
#include "common.h"
#include "dataSend.h"
#include "md5.h"
#include <atomic>
#include <semaphore.h>
#include <thread>
RtspDataManager::RtspDataManager()
{
}

RtspDataManager::~RtspDataManager()
{
}

RtspDataManager *RtspDataManager::getInstance()
{
        static RtspDataManager ins;
        return &ins;
}
namespace
{
        int seq_num = 1;
        bool need_certify = false;
        unsigned int sleep_usectime = 1000;
        std::string NONECE, REALM, RESPONSE, SESSION, TRACK;
        int RTCP_SERVER_PORT = 0;
        std::atomic_bool is_stop = {true};
        sem_t sem_tcpmsg;
#if 0
        const char OPT_MSG[] = "OPTIONS rtsp://192.168.3.18:554/cam/realmonitor?channel=1&subtype=0 RTSP/1.0\r\n";
        const char DESCRIBE_MSG[] = "DESCRIBE rtsp://192.168.3.18:554/cam/realmonitor?channel=1&subtype=0 RTSP/1.0\r\n";
        const char SETUP_MSG[] = "SETUP rtsp://192.168.3.18:554/cam/realmonitor?channel=1&subtype=0/trackID=0 RTSP/1.0\r\n";
        const char PLAY_MSG[] = "PLAY rtsp://192.168.3.18:554/cam/realmonitor?channel=1&subtype=0/ RTSP/1.0\r\n";
        const char STOP_MSG[] = "TEARDOWN rtsp://192.168.3.18:554/cam/realmonitor?channel=1&subtype=0/ RTSP/1.0\r\n";

        const std::string CSEQ_HEAD = "CSeq: "; //后加\r\n
        const std::string AUTHOR_MSG_1 = "Authorization: Digest username=\"admin\", realm=\"";
        const std::string AUTHOR_MSG_2 = "\", nonce=\"";
        const std::string AUTHOR_MSG_3 = "\", uri=\"rtsp://192.168.3.18:554/cam/realmonitor?channel=1&subtype=0\", response=\""; //respose为md5值""
        const std::string AUTHOR_MSG_4 = "\"\r\n";                                                                               //respose为md5值""

        const std::string AUTHOR_MSG_3_2 = "\", uri=\"rtsp://192.168.3.18:554/cam/realmonitor?channel=1&subtype=0/\", response=\""; //respose为md5值""

        const char USER_AGENT_MSG[] = "User-Agent: virtual client player\r\n";
        const char TRANSPORT_MSG[] = "Transport: RTP/AVP;unicast;client_port=9980-9990\r\n"; //rtp端口-rtcp端口

        const char END[] = "\r\n";
        // const char MSG[] = "rtsp://admin:xuehu0930@192.168.3.18:554/cam/realmonitor?channel=1&subtype=0";
#endif
        // rtsp://admin:123@192.168.3.18:554
        const std::string OPT_MSG_1 = "OPTIONS ";
        const std::string DESCRIBE_MSG_1 = "DESCRIBE ";
        const std::string SETUP_MSG_1 = "SETUP ";
        const std::string PLAY_MSG_1 = "PLAY ";
        const std::string STOP_MSG_1 = "TEARDOWN ";

        const std::string CSEQ_HEAD = "CSeq: ";
        const std::string AUTHOR_MSG_1 = "Authorization: Digest username=\"";
        const std::string AUTHOR_MSG_2 = "\", realm=\"";
        const std::string AUTHOR_MSG_3 = "\", nonce=\"";
        const std::string AUTHOR_MSG_4 = "\", uri=\"";
        const std::string AUTHOR_MSG_5 = "\", response=\"";
        const std::string AUTHOR_MSG_6 = "\"\r\n";

        const char USER_AGENT_MSG[] = "User-Agent: virtual client player\r\n";
        const char TRANSPORT_MSG[] = "Transport: RTP/AVP;unicast;client_port=9980-9990\r\n"; //rtp端口-rtcp端口

        const char END[] = "\r\n";
        const char RTSP_END[] = " RTSP/1.0\r\n";

} // namespace

static std::string get_opt_msg(const std::string &url, bool author = false);
static std::string get_describe_msg(const std::string &url, bool author = false);
static std::string get_parameter_msg(const std::string &url);

void RtspDataManager::start()
{
        is_stop = false;
        sem_init(&sem_tcpmsg, 0, 0);

        pThread_tcp = new std::thread(receive_tcp_thread, (void *)this);
}

void RtspDataManager::stop()
{
        sleep_usectime = 1000;
#if 0
        char send_msg[1024];
        memset(send_msg, 0, 1024);
        int pos = 0;
        std::string cseq_str = CSEQ_HEAD + std::to_string(seq_num++) + "\r\n";

        calResponse("TEARDOWN:");
        memcpy(send_msg, STOP_MSG, strlen(STOP_MSG));
        pos += strlen(STOP_MSG);
        memcpy(send_msg + pos, cseq_str.c_str(), cseq_str.size());
        pos += cseq_str.size();
        std::stringstream ostr2;
        ostr2 << AUTHOR_MSG_1 << REALM << AUTHOR_MSG_2 << NONECE << AUTHOR_MSG_3_2 << RESPONSE << AUTHOR_MSG_4;
        memcpy(send_msg + pos, ostr2.rdbuf()->str().c_str(), ostr2.rdbuf()->str().size());
        pos += ostr2.rdbuf()->str().size();
        memcpy(send_msg + pos, USER_AGENT_MSG, strlen(USER_AGENT_MSG));
        pos += strlen(USER_AGENT_MSG);
        std::stringstream ostr5;
        ostr5 << "Session: " << SESSION << "\r\n";
        memcpy(send_msg + pos, ostr5.rdbuf()->str().c_str(), ostr5.rdbuf()->str().size());
        pos += ostr5.rdbuf()->str().size();
        memcpy(send_msg + pos, END, strlen(END));
        pos += strlen(END);

        bool res = pSocketTcp->send(send_msg, pos);
        FSERVO_CHECK(res);
        // sem_wait(&sem_tcpmsg);
#endif
        is_stop = true;

        pThread_tcp->join();
        delete pThread_tcp;
}

void RtspDataManager::send_report_thread(void *arg)
{
        RtspDataManager *pThis = (RtspDataManager *)arg;

        int buffer_len = 32;
        unsigned char *send_buffer = new unsigned char[buffer_len];

        bool is_received = false;
        // uint32_t LSR, DLSR;
        for (;;)
        {
                if (is_stop) return;

                memset(send_buffer, 0, buffer_len);

                //rtp version 2bit
                send_buffer[0] = (send_buffer[0] | pThis->VERSION) << 6;
                //padding 1bit 默认为0,就不写了
                //report count 5bit 一般为1
                send_buffer[0] = send_buffer[0] | 0x01;

                //第2个字节 8bit标识 PT类型，RR为201，SR为202
                send_buffer[1] = (unsigned char)201;
                //3,4字节，length，为头长度减1，即7
                short *plen = (short *)&send_buffer[2];
                *plen = htons((uint16_t)7);
                //SSRC，4字节
                uint32_t *pSSRC = (uint32_t *)&send_buffer[4];
                *pSSRC = htonl((uint32_t)0x00000001);

                //ssrc_n 4字节 rtp包ssrc值
                uint32_t *pSSRC_n = (uint32_t *)&send_buffer[8];
                *pSSRC_n = htonl(pThis->SSRC);

                //fraction lost 8bit 定点数，乘256取整，此处直接设置为0
                send_buffer[12] = 0;
                //cumulative number of packets lost 24bits 直接设置为ff
                send_buffer[13] = 0xff;
                send_buffer[14] = 0xff;
                send_buffer[15] = 0xff;

                //16 bit
                uint16_t *pRound_cout = (uint16_t *)&send_buffer[16];
                *pRound_cout = htons(pThis->wrap_around_count);
                //16bit
                uint16_t *pseq_max = (uint16_t *)&send_buffer[18];
                *pseq_max = htons(pThis->SEQ);
                //interarrival jitter：32 bits
                uint32_t *pjitter = (uint32_t *)&send_buffer[20];
                *pjitter = htonl(80);

                if (false == is_received)
                {
                        is_received = true;
                }
                else if (0 == pThis->SR_Time)
                {
                        const std::string &url = Common::getNetPara().url;
                        std::string str_msg = get_parameter_msg(url);
                        bool res = pThis->pSocketTcp->send(str_msg.c_str(), str_msg.size());
                        FSERVO_CHECK(res);
                }
                else
                {
                        uint32_t *pLSR = (uint32_t *)&send_buffer[24];
                        *pLSR = htonl(pThis->LSR);

                        auto current_time = get_current_Mtime();

                        FSERVO_CHECK(pThis->SR_Time != 0);

                        uint32_t DLSR = (current_time - pThis->SR_Time) * 65536;

                        uint32_t *pDLSR = (uint32_t *)&send_buffer[28];
                        *pDLSR = htonl(DLSR);
                }

                bool res = pThis->pSocketUdp->send((char *)send_buffer, 32);
                FSERVO_CHECK(res);

                usleep(30 * 1000 * 1000);
        }
}

void RtspDataManager::receive_tcp_thread(void *arg)
{
        RtspDataManager *pThis = (RtspDataManager *)arg;

        int len = 1024 * 24;
        char res_buffer[1024 * 24] = {0};

        for (;;)
        {
                if (is_stop) return;

                memset(res_buffer, 0, len);
                int res_len = pThis->pSocketTcp->read(res_buffer, len);

                if (res_len <= 0)
                {
                        if (errno == EWOULDBLOCK || errno == EINTR) //非阻塞模式，ret为-1
                        {
                                usleep(sleep_usectime);
                                continue;
                        }
                        else //连接断开
                        {
                                LOG_WARN("camera disconnected!");
                                sleep(1);
                                // break;
                                continue;
                        }
                }
                else if (res_len > 2)
                {
                        LOG_INFO("receive rtsp server info:\n%s", res_buffer);

                        pThis->parser(res_buffer, res_len);

                        sem_post(&sem_tcpmsg);
                }
        }
}

void RtspDataManager::recv_report_thread(void *arg)
{
        RtspDataManager *pThis = (RtspDataManager *)arg;

        char receive_buffer[100];
        static uint64_t count = 0;
        for (;;)
        {
                if (is_stop) return;

                int ret = pThis->pSocketUdp->read(receive_buffer, 100);
                if (ret <= 0)
                {
                        count++;
                        if (count < 36000)
                                usleep(5 * 1000);
                        else
                        {
                                usleep(60 * 1000 * 1000);
                        }
                        continue;
                }

                uint32_t *pTimeStampMid = (uint32_t *)&receive_buffer[10];
                pThis->LSR = ntohl(*pTimeStampMid);

                pThis->SR_Time = get_current_Mtime();

                // LOG_INFO("receive report msg");
        }
}

#include <regex>
void RtspDataManager::parser(const char *pstr, int len)
{
        static bool isfirst = true;
        static bool isfirst2 = true;
        static bool isfirst3 = true;

        std::string respose_str(pstr, len);

        int begin_pos = respose_str.find("Unauthorized");
        if (begin_pos != std::string::npos && isfirst)
        {
                isfirst = false;
                auto vec = split_string(respose_str, "\r\n");
                need_certify = true;
                for (int i = 0; i < vec.size(); i++)
                {
#if 0
                        auto pos1 = vec[i].find("WWW-Authenticate: Digest realm=\"");
                        if (pos1 != std::string::npos)
                        {
                                auto vec2 = split_string(vec[i], ',');
                                FSERVO_CHECK(vec2.size() == 2);

                                int len = strlen("WWW-Authenticate: Digest realm=\"");
                                int end_pos = vec2[0].find_first_of("\"", len);
                                REALM = vec2[0].substr(len, end_pos - len);

                                int pos2 = vec2[1].find("nonce=\"");
                                end_pos = vec2[1].find_first_of("\"", strlen("nonce=\""));
                                NONECE = vec2[1].substr(strlen("nonce=\""), end_pos - strlen("nonce=\""));

                                LOG_INFO("get realm:%s,nonce:%s", REALM.c_str(), NONECE.c_str());

                                break;
                        }
#endif

                        auto vec2 = split_string(vec[i], ',');
                        for (int i = 0; i < vec2.size(); i++)
                        {
                                auto str = vec2[i];

                                int pos = str.find("realm=\"");
                                if (pos != std::string::npos)
                                {
                                        REALM = str.substr(pos + strlen("realm=\""), str.size() - pos - 1 - strlen("realm=\""));
                                }
                                else
                                {
                                        pos = str.find("nonce=\"");
                                        if (pos != std::string::npos)
                                        {
                                                NONECE = str.substr(pos + strlen("nonce=\""), str.size() - pos - 1 - strlen("nonce=\""));
                                        }
                                }
                        }
                }
                FSERVO_CHECK(!NONECE.empty() && !REALM.empty());
                LOG_INFO("get realm:%s,nonce:%s", REALM.c_str(), NONECE.c_str());
#if 0
                if (vec.size() == 3)
                {
                        std::string &res_str = vec[2];
                        int len = strlen("WWW-Authenticate: Digest realm=\"");
                        int end_pos = res_str.find_first_of("\"", len);
                        REALM = res_str.substr(len, end_pos - len);

                        std::stringstream ostr;
                        ostr << "WWW-Authenticate: Digest realm=\"" << REALM << "\",nonce=\"";
                        len = ostr.rdbuf()->str().size();
                        NONECE = res_str.substr(len, res_str.size() - len - 1);

                        LOG_INFO("get dev:%s,nonce:%s", REALM.c_str(), NONECE.c_str());
                }
#endif
                return;
        }

        begin_pos = respose_str.find("Session: ");
        if (begin_pos != std::string::npos && isfirst2)
        {
                isfirst2 = false;
                auto vec = split_string(respose_str, "\r\n");
                for (int i = 0; i < vec.size(); i++)
                {
#if 1
                        if (vec[i].find("Session: ") == 0)
                        {

                                auto vec2 = split_string(vec[i], ';');
                                int len = strlen("Session: ");
                                SESSION = vec2[0].substr(len, vec2[0].size() - len);

                                LOG_INFO("get session:%s", SESSION.c_str());
                                // break;
                        }
#endif
                        if (vec[i].find("server_port=") != std::string::npos)
                        {
                                auto vec2 = split_string(vec[i], ';');
                                //获取端口
                                for (int k = 0; k < vec2.size(); k++)
                                {
                                        if (vec2[k].find("server_port") == 0)
                                        {
                                                std::string port = split_string(vec2[k], '-')[1];
                                                RTCP_SERVER_PORT = std::atoi(port.c_str());
                                                break;
                                        }
                                }

                                LOG_INFO("get rtcp server port:%d", RTCP_SERVER_PORT);
                                FSERVO_CHECK(RTCP_SERVER_PORT != 0);
                                // break;
                        }
                }
#if 0
                if (vec.size() == 5)
                {
                        SESSION = vec[2].substr(strlen("Session: "), vec[2].size() - strlen("Session: "));

                        std::string port = split_string(split_string(vec[3], ';')[3], '-')[1];

                        RTCP_SERVER_PORT = std::atoi(port.c_str());

                        LOG_INFO("get session:%s,rtcp server port:%d", SESSION.c_str(), RTCP_SERVER_PORT);
                }
#endif
                return;
        }

#if 0
        begin_pos = respose_str.find("control:track");
        if (begin_pos != std::string::npos && isfirst3)
        {
                isfirst3 = false;
                auto vec = split_string(respose_str, "\r\n");
                for (int i = 0; i < vec.size(); i++)
                {
                        if (vec[i].find("control:track") != std::string::npos)
                        {
                                static bool only_video = true;
                                if (only_video)
                                {
                                        TRACK = split_string(vec[i], ':')[1];

                                        LOG_INFO("get track:%s", TRACK.c_str());
                                        only_video = false;

                                        // break;
                                }
                        }
                        else if (vec[i].find("H265") != std::string::npos)
                        {
                                TYPE = CODEC_TYPE::H265;
                        }
                        else if (vec[i].find("H264") != std::string::npos)
                        {
                                TYPE = CODEC_TYPE::H264;
                        }
                }
        }
#endif
        auto key_pos = respose_str.find("v=");
        auto key_pos2 = respose_str.find("a=control");
        auto key_pos3 = respose_str.find("track");
        if (key_pos != std::string::npos && key_pos2 != std::string::npos && key_pos3 != std::string::npos)
        {
                auto vec = split_string(respose_str, "\r\n");
                for (int i = 0; i < vec.size(); i++)
                {
                        auto str = vec[i];
                        if (str.find("a=control") != std::string::npos)
                        {
                                int pos = str.find("track");
                                static bool only_video = true;
                                if (pos != std::string::npos && only_video)
                                {
                                        TRACK = str.substr(pos, str.size() - pos);
                                        LOG_INFO("get track:%s", TRACK.c_str());

                                        only_video = false;
                                }
                        }
                        else if (str.find("H265") != std::string::npos)
                        {
                                TYPE = CODEC_TYPE::H265;
                        }
                        else if (str.find("H264") != std::string::npos)
                        {
                                TYPE = CODEC_TYPE::H264;
                        }
                }
        }
}
#if 0
char *RtspDataManager::calResponse(const char *cmd)
{
        char md5_buffer[1024];
        int pos = 0;
        memset(md5_buffer, 0, 1024);
        memcpy(md5_buffer, "admin:", strlen("admin:"));
        pos += strlen("admin:");
        std::stringstream ostr2;
        ostr2 << REALM << ":";
        // ostr2 << "Login to A23E7E3D381D422B"
        //       << ":";

        memcpy(md5_buffer + pos, ostr2.rdbuf()->str().c_str(), ostr2.rdbuf()->str().size());
        pos += ostr2.rdbuf()->str().size();
        memcpy(md5_buffer + pos, "xuehu0930", strlen("xuehu0930"));
        pos += strlen("xuehu0930");
        char *p_md5_1 = getMd5((unsigned char *)md5_buffer, pos);

        pos = 0;
        // memcpy(md5_buffer, "OPTIONS:", strlen("OPTIONS:"));
        // pos += strlen("OPTIONS:");
        memcpy(md5_buffer, cmd, strlen(cmd));
        pos += strlen(cmd);
        memcpy(md5_buffer + pos, "rtsp://192.168.3.18:554/cam/realmonitor?channel=1&subtype=0", strlen("rtsp://192.168.3.18:554/cam/realmonitor?channel=1&subtype=0"));
        pos += strlen("rtsp://192.168.3.18:554/cam/realmonitor?channel=1&subtype=0");
        char *p_md5_2 = getMd5((unsigned char *)md5_buffer, pos);

        pos = 0;
        memcpy(md5_buffer, p_md5_1, strlen(p_md5_1));
        pos += strlen(p_md5_1);
        memcpy(md5_buffer + pos, ":", 1);
        pos += 1;

        std::stringstream ostr3;
        ostr3 << NONECE << ":";
        // ostr3 << "833eb675-f4d6-4fad-a15d-4d8fabf893fe"
        //       << ":";

        memcpy(md5_buffer + pos, ostr3.rdbuf()->str().c_str(), ostr3.rdbuf()->str().size());
        pos += ostr3.rdbuf()->str().size();
        memcpy(md5_buffer + pos, p_md5_2, strlen(p_md5_2));
        pos += strlen(p_md5_2);

        char *p_md5 = getMd5((unsigned char *)md5_buffer, pos);
        RESPONSE = p_md5;

        free(p_md5);
        free(p_md5_1);
        free(p_md5_2);
}
#endif
static std::string calResponse(const char *cmd, const std::string &user, const std::string passwd, const std::string &url)
{
        char md5_buffer[1024];
        int pos = 0;
        memset(md5_buffer, 0, 1024);

        std::stringstream ostr;
        ostr << user << ":";
        memcpy(md5_buffer, ostr.rdbuf()->str().c_str(), ostr.rdbuf()->str().size());
        pos += ostr.rdbuf()->str().size();

        std::stringstream ostr2;
        ostr2 << REALM << ":";
        memcpy(md5_buffer + pos, ostr2.rdbuf()->str().c_str(), ostr2.rdbuf()->str().size());
        pos += ostr2.rdbuf()->str().size();

        memcpy(md5_buffer + pos, passwd.c_str(), passwd.size());
        pos += passwd.size();

        char *p_md5_1 = getMd5((unsigned char *)md5_buffer, pos);

        pos = 0;
        memcpy(md5_buffer, cmd, strlen(cmd));
        pos += strlen(cmd);

        memcpy(md5_buffer + pos, url.c_str(), url.size());
        pos += url.size();

        char *p_md5_2 = getMd5((unsigned char *)md5_buffer, pos);

        pos = 0;
        memcpy(md5_buffer, p_md5_1, strlen(p_md5_1));
        pos += strlen(p_md5_1);
        memcpy(md5_buffer + pos, ":", 1);
        pos += 1;

        std::stringstream ostr3;
        ostr3 << NONECE << ":";
        memcpy(md5_buffer + pos, ostr3.rdbuf()->str().c_str(), ostr3.rdbuf()->str().size());
        pos += ostr3.rdbuf()->str().size();
        memcpy(md5_buffer + pos, p_md5_2, strlen(p_md5_2));
        pos += strlen(p_md5_2);

        char *p_md5 = getMd5((unsigned char *)md5_buffer, pos);
        RESPONSE = p_md5;

        free(p_md5);
        free(p_md5_1);
        free(p_md5_2);

        return RESPONSE;
}

static std::string get_opt_msg(const std::string &url, bool author)
{
        std::string resStr;
        std::stringstream strStream;

        std::string cseq_str = CSEQ_HEAD + std::to_string(seq_num++) + "\r\n";
        if (author)
        {
                const std::string &user = Common::getNetPara().cameraUser;
                const std::string &passwd = Common::getNetPara().cameraPassword;
                RESPONSE = calResponse("OPTIONS:", user, passwd, url);

                strStream << OPT_MSG_1 << url << RTSP_END << cseq_str
                          << AUTHOR_MSG_1 << user << AUTHOR_MSG_2 << REALM << AUTHOR_MSG_3
                          << NONECE << AUTHOR_MSG_4 << url << AUTHOR_MSG_5 << RESPONSE
                          << AUTHOR_MSG_6 << USER_AGENT_MSG << END;
        }
        else
        {
                strStream << OPT_MSG_1 << url << RTSP_END << cseq_str << USER_AGENT_MSG << END;
        }

        return strStream.rdbuf()->str();
}

static std::string get_describe_msg(const std::string &url, bool author)
{
        std::string resStr;
        std::stringstream strStream;

        std::string cseq_str = CSEQ_HEAD + std::to_string(seq_num++) + "\r\n";
        if (author)
        {
                const std::string &user = Common::getNetPara().cameraUser;
                const std::string &passwd = Common::getNetPara().cameraPassword;
                RESPONSE = calResponse("DESCRIBE:", user, passwd, url);

                strStream << DESCRIBE_MSG_1 << url << RTSP_END << cseq_str
                          << AUTHOR_MSG_1 << user << AUTHOR_MSG_2 << REALM << AUTHOR_MSG_3
                          << NONECE << AUTHOR_MSG_4 << url << AUTHOR_MSG_5 << RESPONSE
                          << AUTHOR_MSG_6 << USER_AGENT_MSG << END;
        }
        else
        {
                strStream << DESCRIBE_MSG_1 << url << RTSP_END << cseq_str << USER_AGENT_MSG << END;
        }

        return strStream.rdbuf()->str();
}

static std::string get_setup_msg(const std::string &url)
{
        std::string resStr;
        std::stringstream strStream;

        std::string cseq_str = CSEQ_HEAD + std::to_string(seq_num++) + "\r\n";

        const std::string &user = Common::getNetPara().cameraUser;
        const std::string &passwd = Common::getNetPara().cameraPassword;
        RESPONSE = calResponse("SETUP:", user, passwd, url);

        FSERVO_CHECK(!TRACK.empty());
        strStream << SETUP_MSG_1 << url << "/" << TRACK << RTSP_END << cseq_str
                  << AUTHOR_MSG_1 << user << AUTHOR_MSG_2 << REALM << AUTHOR_MSG_3
                  << NONECE << AUTHOR_MSG_4 << url << AUTHOR_MSG_5 << RESPONSE
                  << AUTHOR_MSG_6 << USER_AGENT_MSG << TRANSPORT_MSG << END;

        return strStream.rdbuf()->str();
}

static std::string get_play_msg(const std::string &url)
{
        std::string resStr;
        std::stringstream strStream;

        std::string cseq_str = CSEQ_HEAD + std::to_string(seq_num++) + "\r\n";

        const std::string &user = Common::getNetPara().cameraUser;
        const std::string &passwd = Common::getNetPara().cameraPassword;
        RESPONSE = calResponse("PLAY:", user, passwd, url);
        FSERVO_CHECK(!SESSION.empty());
        strStream << PLAY_MSG_1 << url << "/" << RTSP_END << cseq_str
                  << AUTHOR_MSG_1 << user << AUTHOR_MSG_2 << REALM << AUTHOR_MSG_3
                  << NONECE << AUTHOR_MSG_4 << url << AUTHOR_MSG_5 << RESPONSE
                  << AUTHOR_MSG_6 << USER_AGENT_MSG << "Session: " << SESSION << "\r\n"
                  << "Range: npt=0.000-\r\n"
                  << END;

        return strStream.rdbuf()->str();
}

static std::string get_parameter_msg(const std::string &url)
{
        std::string resStr;
        std::stringstream strStream;

        std::string cseq_str = CSEQ_HEAD + std::to_string(seq_num++) + "\r\n";

        const std::string &user = Common::getNetPara().cameraUser;
        const std::string &passwd = Common::getNetPara().cameraPassword;
        RESPONSE = calResponse("GET_PARAMETER:", user, passwd, url);

        // strStream << "GET_PARAMETER " << url << "/"
        //           << "user=" << user << "_"
        //           << "password=" << passwd << "_"
        //           << "channel=0_stream=0.sdp/" << RTSP_END << cseq_str
        //           << AUTHOR_MSG_1 << user << AUTHOR_MSG_2 << REALM << AUTHOR_MSG_3
        //           << NONECE << AUTHOR_MSG_4 << url << AUTHOR_MSG_5 << RESPONSE
        //           << AUTHOR_MSG_6 << USER_AGENT_MSG << "Session: " << SESSION << "\r\n"
        //           << "Range: npt=0.000-\r\n"
        //           << END;
        FSERVO_CHECK(!SESSION.empty());
        strStream << "GET_PARAMETER " << url << "/"
                  << RTSP_END << cseq_str
                  << AUTHOR_MSG_1 << user << AUTHOR_MSG_2 << REALM << AUTHOR_MSG_3
                  << NONECE << AUTHOR_MSG_4 << url << AUTHOR_MSG_5 << RESPONSE
                  << AUTHOR_MSG_6 << USER_AGENT_MSG << "Session: " << SESSION << "\r\n"
                  << END;

        return strStream.rdbuf()->str();
}

void RtspDataManager::login(const char *_dstIp, unsigned short _dstPort, CODEC_TYPE &_type)
{
        pSocketTcp = new SocketTcp;
        const std::string &camera_ip = Common::getNetPara().cameraIP;
        pSocketTcp->connetToServer(camera_ip.c_str(), _dstPort);

        start();
#if 0
        char send_msg[1024];
        memset(send_msg, 0, 1024);
        int pos = 0;
        std::string cseq_str = CSEQ_HEAD + std::to_string(seq_num++) + "\r\n";

        //OPTIONS login
        memcpy(send_msg, OPT_MSG, strlen(OPT_MSG));
        pos += strlen(OPT_MSG);
        memcpy(send_msg + pos, cseq_str.c_str(), cseq_str.size());
        pos += cseq_str.size();
        memcpy(send_msg + pos, USER_AGENT_MSG, strlen(USER_AGENT_MSG));
        pos += strlen(USER_AGENT_MSG);
        memcpy(send_msg + pos, END, strlen(END));
        pos += strlen(END);

        bool res = pSocketTcp->send(send_msg, pos);
        FSERVO_CHECK(res);
        sem_wait(&sem_tcpmsg);

        //OPTIONS Authorization
        pos = 0;
        memset(send_msg, 0, 1024);
        calResponse("OPTIONS:");
        cseq_str = CSEQ_HEAD + std::to_string(seq_num++) + "\r\n";

        memcpy(send_msg, OPT_MSG, strlen(OPT_MSG));
        pos += strlen(OPT_MSG);
        memcpy(send_msg + pos, cseq_str.c_str(), cseq_str.size());
        pos += cseq_str.size();
        std::stringstream ostr;
        ostr << AUTHOR_MSG_1 << REALM << AUTHOR_MSG_2 << NONECE << AUTHOR_MSG_3 << RESPONSE << AUTHOR_MSG_4;
        memcpy(send_msg + pos, ostr.rdbuf()->str().c_str(), ostr.rdbuf()->str().size());
        pos += ostr.rdbuf()->str().size();
        memcpy(send_msg + pos, USER_AGENT_MSG, strlen(USER_AGENT_MSG));
        pos += strlen(USER_AGENT_MSG);
        memcpy(send_msg + pos, END, strlen(END));
        pos += strlen(END);

        res = pSocketTcp->send(send_msg, pos);
        FSERVO_CHECK(res);
        sem_wait(&sem_tcpmsg);

        //DESCRIBE
        pos = 0;
        memset(send_msg, 0, 1024);
        calResponse("DESCRIBE:");
        cseq_str = CSEQ_HEAD + std::to_string(seq_num++) + "\r\n";

        memcpy(send_msg, DESCRIBE_MSG, strlen(DESCRIBE_MSG));
        pos += strlen(DESCRIBE_MSG);
        memcpy(send_msg + pos, cseq_str.c_str(), cseq_str.size());
        pos += cseq_str.size();
        std::stringstream ostr2;
        ostr2 << AUTHOR_MSG_1 << REALM << AUTHOR_MSG_2 << NONECE << AUTHOR_MSG_3 << RESPONSE << AUTHOR_MSG_4;
        memcpy(send_msg + pos, ostr2.rdbuf()->str().c_str(), ostr2.rdbuf()->str().size());
        pos += ostr2.rdbuf()->str().size();
        memcpy(send_msg + pos, USER_AGENT_MSG, strlen(USER_AGENT_MSG));
        pos += strlen(USER_AGENT_MSG);
        memcpy(send_msg + pos, END, strlen(END));
        pos += strlen(END);

        res = pSocketTcp->send(send_msg, pos);
        FSERVO_CHECK(res);
        sem_wait(&sem_tcpmsg);

        //SETUP
        pos = 0;
        memset(send_msg, 0, 1024);
        calResponse("SETUP:");
        cseq_str = CSEQ_HEAD + std::to_string(seq_num++) + "\r\n";

        memcpy(send_msg, SETUP_MSG, strlen(SETUP_MSG));
        pos += strlen(SETUP_MSG);
        memcpy(send_msg + pos, cseq_str.c_str(), cseq_str.size());
        pos += cseq_str.size();
        std::stringstream ostr3;
        ostr3 << AUTHOR_MSG_1 << REALM << AUTHOR_MSG_2 << NONECE << AUTHOR_MSG_3_2 << RESPONSE << AUTHOR_MSG_4;
        memcpy(send_msg + pos, ostr3.rdbuf()->str().c_str(), ostr3.rdbuf()->str().size());
        pos += ostr3.rdbuf()->str().size();
        memcpy(send_msg + pos, USER_AGENT_MSG, strlen(USER_AGENT_MSG));
        pos += strlen(USER_AGENT_MSG);
        memcpy(send_msg + pos, TRANSPORT_MSG, strlen(TRANSPORT_MSG));
        pos += strlen(TRANSPORT_MSG);
        memcpy(send_msg + pos, END, strlen(END));
        pos += strlen(END);

        res = pSocketTcp->send(send_msg, pos);
        FSERVO_CHECK(res);
        sem_wait(&sem_tcpmsg);

        //PLAY
        pos = 0;
        memset(send_msg, 0, 1024);
        calResponse("PLAY:");
        cseq_str = CSEQ_HEAD + std::to_string(seq_num++) + "\r\n";

        memcpy(send_msg, PLAY_MSG, strlen(PLAY_MSG));
        pos += strlen(PLAY_MSG);
        memcpy(send_msg + pos, cseq_str.c_str(), cseq_str.size());
        pos += cseq_str.size();
        std::stringstream ostr4;
        ostr4 << AUTHOR_MSG_1 << REALM << AUTHOR_MSG_2 << NONECE << AUTHOR_MSG_3_2 << RESPONSE << AUTHOR_MSG_4;
        memcpy(send_msg + pos, ostr4.rdbuf()->str().c_str(), ostr4.rdbuf()->str().size());
        pos += ostr4.rdbuf()->str().size();
        memcpy(send_msg + pos, USER_AGENT_MSG, strlen(USER_AGENT_MSG));
        pos += strlen(USER_AGENT_MSG);
        std::stringstream ostr5;
        ostr5 << "Session: " << SESSION << "\r\n";
        memcpy(send_msg + pos, ostr5.rdbuf()->str().c_str(), ostr5.rdbuf()->str().size());
        pos += ostr5.rdbuf()->str().size();
        memcpy(send_msg + pos, "Range: npt=0.000-\r\n", strlen("Range: npt=0.000-\r\n"));
        pos += strlen("Range: npt=0.000-\r\n");
        memcpy(send_msg + pos, END, strlen(END));
        pos += strlen(END);

        res = pSocketTcp->send(send_msg, pos);
        FSERVO_CHECK(res);
        sem_wait(&sem_tcpmsg);
#endif

        const std::string &url = Common::getNetPara().url;

        std::string str_msg = get_opt_msg(url);
        bool res = pSocketTcp->send(str_msg.c_str(), str_msg.size());
        FSERVO_CHECK(res);
        sem_wait(&sem_tcpmsg);

        if (need_certify)
        {
                str_msg = get_opt_msg(url, true);

                res = pSocketTcp->send(str_msg.c_str(), str_msg.size());
                FSERVO_CHECK(res);
                sem_wait(&sem_tcpmsg);
        }

        bool is_certifed = need_certify;
        str_msg = get_describe_msg(url, is_certifed);
        res = pSocketTcp->send(str_msg.c_str(), str_msg.size());
        FSERVO_CHECK(res);
        sem_wait(&sem_tcpmsg);

        if (need_certify && !is_certifed)
        {
                str_msg = get_describe_msg(url, true);
                res = pSocketTcp->send(str_msg.c_str(), str_msg.size());
                FSERVO_CHECK(res);
                sem_wait(&sem_tcpmsg);
        }

        str_msg = get_setup_msg(url);
        res = pSocketTcp->send(str_msg.c_str(), str_msg.size());
        FSERVO_CHECK(res);
        sem_wait(&sem_tcpmsg);

        _type = TYPE;
        DataSend::getInstance()->set_streamInfo(_type == CODEC_TYPE::H264 ? 0 : 1);
        LOG_INFO("rtsp server login success!");

#if 0
        str_msg = get_play_msg(url);
        res = pSocketTcp->send(str_msg.c_str(), str_msg.size());
        FSERVO_CHECK(res);
        sem_wait(&sem_tcpmsg);

        LOG_INFO("rtsp server init success! start send rtp pakcet");
        sleep_usectime = 10 * 1000 * 1000;

        // stop();
        usleep(100 * 1000);
        pSocketUdp = new SocketUdp;
        pSocketUdp->createUdpClient(camera_ip.c_str(), RTCP_SERVER_PORT, 9990);
        pThread_report_recv = new std::thread(recv_report_thread, (void *)this);
        pThread_report_send = new std::thread(send_report_thread, (void *)this);

#endif
}

void RtspDataManager::play()
{
        const std::string &url = Common::getNetPara().url;
        const std::string &camera_ip = Common::getNetPara().cameraIP;

        std::string str_msg = get_play_msg(url);
        bool res = pSocketTcp->send(str_msg.c_str(), str_msg.size());
        FSERVO_CHECK(res);
        sem_wait(&sem_tcpmsg);

        LOG_INFO("rtsp server play success!");
        sleep_usectime = 10 * 1000 * 1000;

        usleep(100 * 1000);
        pSocketUdp = new SocketUdp;
        FSERVO_CHECK(RTCP_SERVER_PORT != 0);
        res = pSocketUdp->createUdpClient(camera_ip.c_str(), RTCP_SERVER_PORT, 9990);
        FSERVO_CHECK(res);
        // LOG_INFO("camera ip:%s,port:%d",camera_ip.c_str(), RTCP_SERVER_PORT);

        pThread_report_recv = new std::thread(recv_report_thread, (void *)this);
        pThread_report_send = new std::thread(send_report_thread, (void *)this);
}