#pragma once

#include "log.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include "dataModel.hpp"
#ifdef _Unix
#include <sys/time.h>
#include <unistd.h>
#ifdef _NEON
#include <arm_neon.h>
#endif
#endif

struct my_compare
{
        bool operator()(const std::string &key1, const std::string &key2) const
        {
                return true;
        }
};

template <class T>
T stringToNum(const std::string &str)
{
        std::istringstream iss(str);
        T num;
        iss >> num;
        return num;
}

inline int split_string_as_uint32(const std::string &src, std::vector<uint32_t> &dst, const char flag)
{
        std::string tmp;
        std::istringstream is(src);
        char *endptr;
        while (getline(is, tmp, flag))
        {
                // check if decimal
                uint32_t result = std::strtoul(tmp.c_str(), &endptr, 10);
                if (*endptr == '\0')
                {
                        dst.push_back(result);
                        continue;
                }
                // check if octal
                result = std::strtoul(tmp.c_str(), &endptr, 8);
                if (*endptr == '\0')
                {
                        dst.push_back(result);
                        continue;
                }
                // check if hex
                result = std::strtoul(tmp.c_str(), &endptr, 16);
                if (*endptr == '\0')
                {
                        dst.push_back(result);
                        continue;
                }
        }
        return 0;
}

inline uint32_t hexToInt(const std::string &str)
{
        std::string strNum = str.substr(2, str.length() - 2);
        uint32_t res = 0;
        for (size_t i = 0; i < strNum.size(); i++)
        {
                auto c = strNum[i];
                if (c >= 'A' && c <= 'F')
                        res = res * 16 + c - 'A' + 10;
                else if (c >= 'a' && c <= 'f')
                        res = res * 16 + c - 'a' + 10;
                else if (c >= '0' && c <= '9')
                        res = res * 16 + c - '0';
        }
        return res;
}

inline std::string intToHex(int num)
{
        std::string res;
        while (num)
        {
                int n = num % 16;
                char c;
                if (n >= 10)
                        c = 'A' + n - 10;
                else
                        c = '0' + n;
                res.insert(res.begin(), c);
                num /= 16;
        }
        return res;
}

inline std::vector<std::string> string_split(const std::string &str, const char splitStr)
{
        std::vector<std::string> vecStr;
        std::string::size_type beginPos = str.find_first_not_of(splitStr, 0);  //查找起始位置，即第一个不是分隔符的字符的位置
        std::string::size_type endPos = str.find_first_of(splitStr, beginPos); //根据起始位置，查找第一个分隔符位置　
        while (std::string::npos != endPos || std::string::npos != beginPos)
        {                                                                  //开始查找，未找到返回npos,此处条件考虑到尾部结束为分割符的情况以及头部开始就是分隔符并且只有这一个分割符的情况
                vecStr.push_back(str.substr(beginPos, endPos - beginPos)); //从指定位置截断指定长度的字符串，str本身不变
                beginPos = str.find_first_not_of(splitStr, endPos);        //再从上次截断地方开始，查找下一个不是分隔符的起始位置
                endPos = str.find_first_of(splitStr, beginPos);            //再次开始从指定位置查找下一个分隔符位置
        }

        return vecStr;
}

inline std::vector<std::string> split_string(const std::string &str, const char splitStr)
{
        std::vector<std::string> vecStr;
        std::string::size_type beginPos = str.find_first_not_of(splitStr, 0);  //查找起始位置，即第一个不是分隔符的字符的位置
        std::string::size_type endPos = str.find_first_of(splitStr, beginPos); //根据起始位置，查找第一个分隔符位置　
        while (std::string::npos != endPos || std::string::npos != beginPos)
        {                                                                  //开始查找，未找到返回npos,此处条件考虑到尾部结束为分割符的情况以及头部开始就是分隔符并且只有这一个分割符的情况
                vecStr.push_back(str.substr(beginPos, endPos - beginPos)); //从指定位置截断指定长度的字符串，str本身不变
                beginPos = str.find_first_not_of(splitStr, endPos);        //再从上次截断地方开始，查找下一个不是分隔符的起始位置
                endPos = str.find_first_of(splitStr, beginPos);            //再次开始从指定位置查找下一个分隔符位置
        }

        return vecStr;
}

inline std::vector<std::string> split_string(const std::string &str, const char *splitStr)
{
        std::vector<std::string> vecStr;
        std::string::size_type beginPos = str.find_first_not_of(splitStr, 0);  //查找起始位置，即第一个不是分隔符的字符的位置
        std::string::size_type endPos = str.find_first_of(splitStr, beginPos); //根据起始位置，查找第一个分隔符位置　
        while (std::string::npos != endPos || std::string::npos != beginPos)
        {                                                                  //开始查找，未找到返回npos,此处条件考虑到尾部结束为分割符的情况以及头部开始就是分隔符并且只有这一个分割符的情况
                vecStr.push_back(str.substr(beginPos, endPos - beginPos)); //从指定位置截断指定长度的字符串，str本身不变
                beginPos = str.find_first_not_of(splitStr, endPos);        //再从上次截断地方开始，查找下一个不是分隔符的起始位置
                endPos = str.find_first_of(splitStr, beginPos);            //再次开始从指定位置查找下一个分隔符位置
        }

        return vecStr;
}

inline uint64_t get_current_Mtime()
{
#ifdef _Unix
        using namespace std;

        timeval tv;
        gettimeofday(&tv, nullptr);

        //uint64_t time=tv.tv_sec;// 秒
        uint64_t time = tv.tv_sec * 1000 + tv.tv_usec / 1000; //毫秒
        //tv.tv_sec*1000000+tv.tv_usec 微秒
        //uint64_t time=tv.tv_sec*1000+tv.tv_usec/1000;

        return time;
#endif
        return QDateTime::currentMSecsSinceEpoch();
}

#ifdef _NEON
inline void neon_memcpy(volatile void *dst, volatile void *src, int sz)
{
        if (sz & 63)
                sz = (sz & -64) + 64;
        asm volatile(
            "NEONCopyPLD: \n"
            " VLDM %[src]!,{d0-d7} \n"
            " VSTM %[dst]!,{d0-d7} \n"
            " SUBS %[sz],%[sz],#0x40 \n"
            " BGT NEONCopyPLD \n"
            : [dst] "+r"(dst), [src] "+r"(src), [sz] "+r"(sz)
            :
            : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "cc", "memory");
}
#endif

inline std::string get_current_dir()
{
#ifdef _Unix
        char buf[1024] = {0};
        getcwd(buf, 1024);

        return std::string(buf);
#else

        char exePath[1024];
        GetModuleFileName(NULL, exePath, 1023);

        //去掉执行的文件名。
        (strrchr(exePath, '\\'))[1] = 0;
        return std::string(exePath);

#endif

        //return QCoreApplication::applicationDirPath().toStdString();
}

inline bool isFileExist(const char *_file)
{
        return QFile::exists(_file);
}

inline char *readBinaryFile(const char *_file, int &length)
{ //需要手动释放内存
        char *pRes = nullptr;

        std::ifstream i_f_stream(_file, std::ifstream::binary);
        FSERVO_CHECK(i_f_stream.is_open());

        i_f_stream.seekg(0, i_f_stream.end);
        length = i_f_stream.tellg();
        i_f_stream.seekg(0, i_f_stream.beg);

        pRes = new char[length];
        i_f_stream.read(pRes, length);

        i_f_stream.close();

        return pRes;
}

inline char *readBinaryFile2(const char *_file)
{ //无需手动释放内存

        std::ifstream i_f_stream(_file, std::ifstream::binary);
        FSERVO_CHECK(i_f_stream.is_open());

        i_f_stream.seekg(0, i_f_stream.end);
        int length = i_f_stream.tellg();
        i_f_stream.seekg(0, i_f_stream.beg);

        std::shared_ptr<char> pRes(new char[length](), std::default_delete<char[]>());
        i_f_stream.read(pRes.get(), length);

        i_f_stream.close();

        return pRes.get();
}

inline uint64_t rdtsc()
{
#ifdef _ARM64
        int64_t virtual_timer_value;
        asm volatile("mrs %0, cntvct_el0"
                     : "=r"(virtual_timer_value));
        return virtual_timer_value;
#elif defined(__ARM_ARCH)
#if (__ARM_ARCH >= 6) // V6 is the earliest arch that has a standard cyclecount
        uint32_t pmccntr;
        uint32_t pmuseren;
        uint32_t pmcntenset;
        // Read the user mode perf monitor counter access permissions.
        asm volatile("mrc p15, 0, %0, c9, c14, 0"
                     : "=r"(pmuseren));
        if (pmuseren & 1)
        { // Allows reading perfmon counters for user mode code.
                asm volatile("mrc p15, 0, %0, c9, c12, 1"
                             : "=r"(pmcntenset));
                if (pmcntenset & 0x80000000ul)
                { // Is it counting?
                        asm volatile("mrc p15, 0, %0, c9, c13, 0"
                                     : "=r"(pmccntr));
                        // The counter is set up to count every 64th cycle
                        return static_cast<int64_t>(pmccntr) * 64; // Should optimize to << 6
                }
        }
#endif
#else
        unsigned int lo, hi;
        __asm__ __volatile__("rdtsc"
                             : "=a"(lo), "=d"(hi));
        return ((uint64_t)hi << 32) | lo;
#endif
}

inline uint64_t clock_msec()
{
        timespec time1;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
        return time1.tv_sec * 1000 + time1.tv_nsec / 1000000;
}

class Common
{

private:
        Common() = delete;
        ~Common() = delete;

public:
        static void initLog(LOG_LEVEL level);
        static void setCfg(const std::string &key, const std::map<std::string, std::string> &val);
        static void setCfg(const std::string &key, const std::pair<std::string, std::string> &val);
        static bool initCfg(bool containNet = false);
        static const PicParaCfg &getPicCfg();
        static const BasicParaCfg &getBasicPara();
        static const FpgaParaCfg &getFpgaPara();
        static const DetectParaCfg &getDetectPara();
        static const NetParaCfg &getNetPara();

public:
        //static image **getAlphabet();
        //static char  **getClassName();
        static bool isSavePicBox();
};