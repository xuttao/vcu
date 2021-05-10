#include "fpgaControl.h"
#include "common.h"
#include "fileManager.h"
#include "fpgaDecode.h"
#include "log.h"
#include "shm.h"
#ifdef _Unix
#include <unistd.h>
#endif

using namespace snowlake;

namespace
{
        struct CalModel
        {
                unsigned char *output_ptr;
                FPGAInput *input_ptr;

                CalModel(unsigned char *_outptr, FPGAInput *_inputptr) : output_ptr(_outptr), input_ptr(_inputptr) {}
        };

        std::vector<uint32_t> vecInputOffset;
        std::vector<uint32_t> vecOutputOffset;

        std::vector<QSemaphore *> vecSemControl;
        QSemaphore semInput;

        std::mutex dataMutex;
        std::mutex semMutex;
        std::mutex calcuMutex;
        std::condition_variable semCond;
        QSemaphore sem_input;
        QSemaphore sem_calcu;

        std::vector<uint32_t> vecStartIndex;
        std::vector<uint32_t> vecInputIndex;
        std::vector<uint32_t> vecOutputIndex;
        std::vector<uint32_t> vecCalcuateIndex;

        std::vector<std::queue<FPGAInput *>> vecQueInput;

        std::queue<CalModel> que_calcuate;

        std::vector<std::string> vecInteruptDev;
        std::vector<uint32_t> vecInputReg;
        std::vector<uint32_t> vecOutputReg;

        std::atomic_bool isInit(false);
        uint32_t FPGA_CFG_OUTPUT_SIZE = 0;
} // namespace

dev_init_param_st FPGAControl::init_param = {
    .mem_dev_name = FPGA_DEV_MEM,
    .reg_dev_name = FPGA_DEV_REG,
};

char *FPGAControl::mapped_mem_addr = NULL;
#if defined(OPT_BUILD_ARM64_ENABLE)
char *FPGAControl::mapped_mem_addr2 = NULL;
#endif

FPGAControl::~FPGAControl()
{
        isStop = true;
        sem_input.release();
        pThread_Output->join();
        for (int i = 0; i < vecSemControl.size(); ++i)
        {
                delete[] vecSemControl[i];
        }
        delete pThread_Output;

        sem_calcu.release();
        if (!pThread_Calcuate)
        {
                pThread_Calcuate->join();
                delete pThread_Calcuate;
        }
        dev_op_info.close();
}

FPGAControl::FPGAControl()
{
        FSERVO_CHECK(isInit == false);
        isInit = true;
        m_output_wait_time = Common::getFpgaPara().outputWaitTime;
        isStop = false;
        memset(&dev_op_info, 0, sizeof(struct _dev_op_));
        FPGA_CFG_OUTPUT_SIZE = Common::getFpgaPara().outputSize;
}

/************************************************************/

void FPGAControl::input(FPGAInput *input)
{
        int index = 0;
        // vecSemControl[index][vecInputIndex[index] % FPGA_CFG_IO_DATA_MAX_NUM].acquire(); //阻塞

        semInput.acquire();

        dataMutex.lock();
        vecQueInput[index].push(input);
        dataMutex.unlock();

        uint64_t offset = (uint64_t)vecInputOffset[index] + (uint64_t)(FPGA_CFG_INPUT_DATA_SIZE *
                                                                       ((vecInputIndex[index] + vecStartIndex[index]) % FPGA_CFG_IO_DATA_MAX_NUM));
        int32 flag = dev_op_info.write_size((void *)offset, input->in_data,
                                            input->in_data_len, snowlake::DATA_TYPE_DATA, vecInputReg[index], (index > 0 ? true : false));

        sem_input.release();
        ++vecInputIndex[index];
}

void *FPGAControl::output_thread(void *pParam)
{
        FPGAControl *pFpgaControl = static_cast<FPGAControl *>(pParam);
        int index = 0;
        LOG_INFO("fpga output thread start! id:%u", std::this_thread::get_id());

        for (;;)
        {
                if (pFpgaControl->isStop)
                        return nullptr;

                sem_input.acquire();

                int read_count = 0;
                int32_t reg_data;

                for (;;)
                {
                        if (pFpgaControl->isStop)
                                return nullptr;

                        read_count++;
                        if (read_count == 100)
                        {
                                LOG_ERR("interrupt fail!!!");
                                exit(-1);
                        }

                        pFpgaControl->dev_op_info.read_reg(vecOutputReg[index], reg_data);

                        if ((uint32)reg_data - vecStartIndex[index] - vecOutputIndex[index] > 0)
                                break;
#ifdef _Unix
                        usleep(pFpgaControl->m_output_wait_time);
                        //usleep(9 * 1000);
#else
                        std::this_thread::sleep_for(chrono::microseconds(pFpgaControl->m_output_wait_time));
#endif
                }

                //usleep(7*1000);

                uint64_t offset = (uint64_t)vecOutputOffset[index] + (uint64_t)(FPGA_CFG_OUTPUT_SIZE * ((vecOutputIndex[index] + vecStartIndex[index]) % FPGA_CFG_IO_DATA_MAX_NUM));
                unsigned char *pl_address = (unsigned char *)(mapped_mem_addr + offset);

                FPGAOutput fpgaOutput = {pl_address, 0};

                dataMutex.lock();
                FPGAInput *data_info = vecQueInput[index].front();
                vecQueInput[index].pop();
                dataMutex.unlock();

                // vecSemControl[index][vecOutputIndex[index] % FPGA_CFG_IO_DATA_MAX_NUM].release();
                semInput.release();

                pFpgaControl->pCallBackFun(data_info, &fpgaOutput);
                delete data_info;

                ++vecOutputIndex[index];
        }

        LOG_INFO("fpga output thread exit");
}

void *FPGAControl::calcuate_thread(void *pParam)
{
        LOG_INFO("calcuate thread start! id:%u", std::this_thread::get_id());

        FPGAControl *pThis = static_cast<FPGAControl *>(pParam);

        int index = 0;
        for (;;)
        {
                sem_calcu.acquire();

                if (pThis->isStop) return nullptr;

                calcuMutex.lock();
                auto cal_data = que_calcuate.front();
                que_calcuate.pop();
                calcuMutex.unlock();

                FPGAOutput fpgaOutput = {cal_data.output_ptr, 0};
                FPGAInput *fpga_input_ptr = cal_data.input_ptr;

                pThis->pCallBackFun(fpga_input_ptr, &fpgaOutput);

                // delete[] fpga_input_ptr->in_data;
                delete fpga_input_ptr;
        }

        LOG_INFO("calcuate thread exit");
}

void FPGAControl::start()
{
        pThread_Output = new std::thread(output_thread, (void *)this);
        // pThread_Calcuate = new std::thread(calcuate_thread, (void *)this);
}

void FPGAControl::stop()
{
        isStop = true;
}

void FPGAControl::initFpga()
{
        initDev();
        initParam();
        initIndex();
}

void FPGAControl::setCallBackFun(callback_func pf)
{
        pCallBackFun = pf;
}

void FPGAControl::initDev()
{
        int32 ret = ERR_NONE;

        const dev_op_st *dev_ptr = dev_init(DEV_NAME_DEFAULT);
        FSERVO_CHECK(nullptr != dev_ptr);

        ret = dev_ptr->open(init_param);
        FSERVO_CHECK(ERR_NONE == ret);

        dev_op_info = *dev_ptr;

        ret = dev_op_info.get_mapped_mem_addr((void **)&(mapped_mem_addr), false);
        FSERVO_CHECK(ERR_NONE == ret);
#if defined(OPT_BUILD_ARM64_ENABLE)
        ret = dev_op_info.get_mapped_mem_addr((void **)&(mapped_mem_addr2), true);
        FSERVO_CHECK(ERR_NONE == ret);
#endif
}

void FPGAControl::initParam()
{
        RETSTATUS ret = RETSTATUS::RET_NORMAL;
        std::vector<std::string> vecParam_files, vecParam_addrs;

        const FpgaParaCfg &fpgaPara = Common::getFpgaPara();

        vecParam_files = split_string(fpgaPara.paramFiles, ',');
        vecParam_addrs = split_string(fpgaPara.paramAddress, ',');
        FSERVO_CHECK(vecParam_files.size() == vecParam_addrs.size());

        dev_op_st *dev_op_ptr = &dev_op_info;
        for (int i = 0; i < vecParam_files.size(); ++i)
        {
                const char *param_path = vecParam_files[i].c_str();
                if (!isFileExist(param_path))
                {
                        LOG_ERR("Invalid path:%s", param_path);
                        ret = RETSTATUS::RET_DCU_FILE_PATH_INVALID;
                        break;
                }
                int length = 0;
                char *param_buf = readBinaryFile(param_path, length);

                // if (vecParam_files.size() > 2 && i == 2)
                // {
                //         dev_op_ptr->write((void *)stoul(vecParam_addrs[i]), param_buf, length, DATA_TYPE_PARAM, true);
                // }
                // else
                dev_op_ptr->write((void *)stoul(vecParam_addrs[i], nullptr, 16), param_buf, length, DATA_TYPE_PARAM, false);

                LOG_INFO("ddr:0x%x; file:%s; len: %d",
                         stoul(vecParam_addrs[i], nullptr, 16), param_path, length);

                if (param_buf) delete[] param_buf;
        }
}

void FPGAControl::initIndex()
{
        const FpgaParaCfg &fpgaPara = Common::getFpgaPara();

        split_string_as_uint32(fpgaPara.inputOffset, vecInputOffset, ',');
        split_string_as_uint32(fpgaPara.outputOffset, vecOutputOffset, ',');

        FSERVO_CHECK(vecInputOffset.size() > 0 && vecInputOffset.size() == vecOutputOffset.size());

        for (int i = 0; i < vecInputOffset.size(); i++)
                LOG_INFO("input_offset: 0x%x, output_offset: 0x%x, output_size: 0x%x;",
                         vecInputOffset[i], vecOutputOffset[i], FPGA_CFG_OUTPUT_SIZE);

        for (int i = 0; i < vecInputOffset.size(); i++)
        { //单batch size 就是1

                // QSemaphore *pControl = new QSemaphore[FPGA_CFG_IO_DATA_MAX_NUM];

                // for (int n = 0; n < FPGA_CFG_IO_DATA_MAX_NUM; n++)
                //         pControl[n].release(1);

                // vecSemControl.push_back(pControl);

                semInput.release(2);

                vecStartIndex.push_back(0);
                vecInputIndex.push_back(0);
                vecOutputIndex.push_back(0);

                std::queue<FPGAInput *> queInput;
                vecQueInput.push_back(queInput);
        }

        vecInteruptDev = split_string(fpgaPara.interuptDev, ',');
        FSERVO_CHECK(vecInteruptDev.size() == vecInputOffset.size());

        split_string_as_uint32(fpgaPara.inputReg, vecInputReg, ',');
        FSERVO_ASSERT(vecInputReg.size() == vecInputOffset.size());
        split_string_as_uint32(fpgaPara.outputReg, vecOutputReg, ',');
        FSERVO_ASSERT(vecOutputReg.size() == vecInputOffset.size());

        int reg_data = 0;
        fpgaCalStartNum = 0;
        dev_op_st *dev_op_ptr = &dev_op_info;
        for (int i = 0; i < vecOutputReg.size(); ++i)
        {                                                        //单batch 下 size 就为 1
                dev_op_ptr->read_reg(vecOutputReg[i], reg_data); //读取起始fpga reg口的 index 数据
                vecStartIndex[i] = (uint32)reg_data;

                fpgaCalStartNum += reg_data;
        }
}

void FPGAControl::reInit()
{
        vecStartIndex.clear();
        vecInputIndex.clear();
        vecOutputIndex.clear();

        while (!vecQueInput[0].empty())
        {
                usleep(1 * 1000);
        }

        vecQueInput.clear();

        for (int i = 0; i < vecInputOffset.size(); i++)
        { //单batch size 就是1

                for (int n = 0; n < FPGA_CFG_IO_DATA_MAX_NUM; n++)
                        vecSemControl[i][n].release(1);

                vecStartIndex.push_back(0);
                vecInputIndex.push_back(0);
                vecOutputIndex.push_back(0);

                std::queue<FPGAInput *> queInput;
                vecQueInput.push_back(queInput);
        }

        int reg_data = 0;
        fpgaCalStartNum = 0;
        dev_op_st *dev_op_ptr = &dev_op_info;
        for (int i = 0; i < vecOutputReg.size(); ++i)
        {                                                        //单batch 下 size 就为 1
                dev_op_ptr->read_reg(vecOutputReg[i], reg_data); //读取起始fpga reg口的 index 数据
                vecStartIndex[i] = (uint32)reg_data;

                fpgaCalStartNum += reg_data;
        }
}