#pragma once
#include <iostream>
#include <map>
#include <memory>
#include <string.h>
enum class RETSTATUS
{
        //成功
        RET_NORMAL = 0,
        //函数参数传递错误
        RET_FUNC_ERR,
        //buffer已经存满数据，五空余的buffer可用
        RET_BUF_FULL,
        //DCU未初始化
        RET_DCU_NOT_INIT,
        //DCU初始化失败
        RET_DCU_INIT_FAILED,
        //DCU配置文件路径是无效的
        RET_DCU_FILE_PATH_INVALID,
        //DCU配置文件不能打开
        RET_DCU_CFG_FILE_OPEN_FAILED,
        //DCU配置文件不能读取
        RET_DCU_CFG_FILE_READ_FAILED,
        //DCU申请空间失败
        RET_DCU_MALLOC_FAILED
};

const std::map<RETSTATUS, std::string> mpRetStr = {
    std::pair<RETSTATUS, std::string>(RETSTATUS::RET_NORMAL, "normal return."),
    std::pair<RETSTATUS, std::string>(RETSTATUS::RET_FUNC_ERR, "function param error"),
    std::pair<RETSTATUS, std::string>(RETSTATUS::RET_BUF_FULL, "fpga buffer is full"),
    std::pair<RETSTATUS, std::string>(RETSTATUS::RET_DCU_NOT_INIT, "dcu not initialize"),
    std::pair<RETSTATUS, std::string>(RETSTATUS::RET_DCU_INIT_FAILED, "dcu initialized failed"),
    std::pair<RETSTATUS, std::string>(RETSTATUS::RET_DCU_FILE_PATH_INVALID, "the file path is invalid"),
    std::pair<RETSTATUS, std::string>(RETSTATUS::RET_DCU_CFG_FILE_OPEN_FAILED, "can't open file"),
    std::pair<RETSTATUS, std::string>(RETSTATUS::RET_DCU_CFG_FILE_READ_FAILED, "can't read file"),
    std::pair<RETSTATUS, std::string>(RETSTATUS::RET_DCU_MALLOC_FAILED, "dcu malloc failed"),
};

struct FpgaParaCfg
{
        std::string paramFiles;
        std::string paramAddress;
        std::string keyFiles;
        uint32_t inputSize;
        std::string inputOffset;
        std::string outputOffset;
        uint32_t outputSize;
        uint8_t outputSigType;
        uint32_t outputWaitTime;
        std::string inputReg;
        std::string outputReg;
        std::string interuptDev;

        bool isValid() const
        {
                return (paramFiles.empty() || paramAddress.empty() || inputOffset.empty() || outputOffset.empty() || inputSize < 0 || outputSize < 0);
        }
};

struct PicParaCfg
{
        uint8_t processMode;
        double inputScale;
        float inputZero;
        float rMean;
        float gMean;
        float bMean;
        float rStd;
        float gStd;
        float bStd;
};

struct BasicParaCfg
{
        bool savePic;
        bool saveBox;
        std::string picDir;
};

struct DetectParaCfg
{
        std::string netFile;
        std::string labelPath;
        std::string classFile;
};

struct NetParaCfg
{
        std::string url;

        std::string cameraIP;
        int cameraPort;
        std::string cameraUser;
        std::string cameraPassword;

        std::string rtpSendIP;
        int rtpSendPort;

        std::string boxSendIP;
        int boxSendPort;
};

struct FpgaBoxInfo
{
        std::string label;
        int x1 = 0;
        int y1 = 0;
        int x2 = 0;
        int y2 = 0;
        float score = 0;

        FpgaBoxInfo(std::string &&_name, int _x1, int _y1, int _x2, int _y2, float _score) : label(_name), x1(_x1), y1(_y1), x2(_x2), y2(_y2), score(_score) {}
        FpgaBoxInfo()
        {
                memset(this, 0, sizeof(FpgaBoxInfo));
        }
};

struct RgbData
{
        char *pRgb;
        int rgbSize;
        uint32_t picID;
};
