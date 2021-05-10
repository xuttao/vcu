#ifndef FSERVO_FPGAOPT_H
#define FSERVO_FPGAOPT_H

#include "config.h"
#include "dataModel.hpp"
#include "devManager.h"
#include "fservo.h"
#include <QSemaphore>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

struct SemControl
{
        QSemaphore sem_input;
};

class FPGAControl
{
public:
        FPGAControl();
        virtual ~FPGAControl();

public:
        void initFpga();
        void start();
        void stop();
        void setCallBackFun(callback_func pf);
        void input(FPGAInput *input);
        void reInit();

        inline uint32_t get_input_w() { return m_input_w; }
        inline uint32_t get_input_h() { return m_input_h; }

private:
        void initDev();
        void initParam();
        void initIndex();

        static void *output_thread(void *);
        static void *calcuate_thread(void *);

private:
        callback_func pCallBackFun;
        uint32_t fpgaCalStartNum; //fpga起始运行时候的reg口数字,每出一个数,reg数字递增
        std::thread *pThread_Output;
        std::thread *pThread_Calcuate = nullptr;
        std::atomic_bool isStop;

        dcu_init_status_en dcu_init_status;
        uint32_t m_output_wait_time;
        uint32_t m_input_w, m_input_h;

        dev_op_st dev_op_info;
        static dev_init_param_st init_param;

private:
        static char *mapped_mem_addr;
#if defined(OPT_BUILD_ARM64_ENABLE)
        static char *mapped_mem_addr2;
#endif
};

#endif //FSERVO_FPGAOPT_H
