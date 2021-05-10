#pragma once
#if 0
#include <map>

struct FpgaParaCfg
{
        std::string paramFiles;
        std::string paramAddress;
        std::string keyFiles;
        uint32_t inputSize;
        uint32_t inputOffset;
        uint32_t outputOffset;
        uint32_t outputSize;
        uint32_t inputBufferSize;
        uint32_t inputBufferNum;
        uint8_t outputSigType;
        uint32_t outputWaitTime;
        uint32_t inputReg;
        uint32_t outputReg;
        std::string interuptDev;

        bool isValid() const
        {
                return !(paramFiles.empty() || paramAddress.empty());
        }
};

struct PicParaCfg
{
        uint8_t processMode = 0;
        double inputScale = 1.f;
        float inputZero = 0.f;
        float rMean = 0.f;
        float gMean = 0.f;
        float bMean = 0.f;
        float rStd = 0.f;
        float gStd = 0.f;
        float bStd = 0.f;
};

struct DetectParaCfg
{
        std::string netFile;
        std::string labelPath;
        std::string classFile;
};

class ParamManager
{
private:
        ParamManager() = default;
        ~ParamManager() = default;

public:
        static ParamManager *getInstance();
        void parserCfg(const char *_file);
        void initParam();
        inline const FpgaParaCfg &getFpgaCfg() const { return fpgaParam; }
        inline const PicParaCfg &getPicCfg() const { return picParam; }
        inline const DetectParaCfg &getNetCfg() const { return netParam; }

private:
        void setParam(const std::string &group, const std::string &key, const std::string &val);

private:
        std::map<std::string, std::string> mpFpga;
        std::map<std::string, std::string> mpPic;
        std::map<std::string, std::string> mpNet;

        FpgaParaCfg fpgaParam;
        PicParaCfg picParam;
        DetectParaCfg netParam;
};
#endif