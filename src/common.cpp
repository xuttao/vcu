#include "common.h"

namespace
{
        const std::string BasicCfgkey = "basic_cfg";
        const std::string FpgaCfgkey = "fpga_cfg";
        const std::string PicCfgkey = "pic_cfg";
        const std::string Detectkey = "detect_cfg";

        const std::string CameraCfgKey = "camera";
        const std::string RtpSendKey = "rtp_send";
        const std::string BoxSendKey = "box_send";

        FpgaParaCfg fpgaPara;
        PicParaCfg picPara;
        DetectParaCfg detectPara;
        BasicParaCfg basicPara;
        NetParaCfg netPara;

        std::map<std::string, std::string> mpBasic;
        std::map<std::string, std::string> mpFpga;
        std::map<std::string, std::string> mpPic;
        std::map<std::string, std::string> mpDetect;
        std::map<std::string, std::string> mpCamera;
        std::map<std::string, std::string> mpRtp;
        std::map<std::string, std::string> mpBox;
} // namespace

void Common::initLog(LOG_LEVEL level)
{
        logLevel = level;
}

void Common::setCfg(const std::string &key, const std::map<std::string, std::string> &val)
{
        if (key == BasicCfgkey)
        {
                mpBasic = val;
        }
        else if (key == FpgaCfgkey)
        {
                mpFpga = val;
        }
        else if (key == PicCfgkey)
        {
                mpPic = val;
        }
        else if (key == Detectkey)
        {
                mpDetect = val;
        }
        else
        {
                LOG_WARN("no match key:%s", key.c_str());
        }
}

void Common::setCfg(const std::string &key, const std::pair<std::string, std::string> &val)
{
        if (key == CameraCfgKey)
        {
                mpCamera.insert(val);
        }
        else if (key == RtpSendKey)
        {
                mpRtp.insert(val);
        }
        else if (key == BoxSendKey)
        {
                mpBox.insert(val);
        }
        else
        {
                LOG_WARN("no match key:%s", key.c_str());
        }
}

bool Common::initCfg(bool containNet)
{
        fpgaPara.paramFiles = mpFpga["param_files_path"];
        fpgaPara.paramAddress = mpFpga["param_files_addrs"];
        fpgaPara.keyFiles = mpFpga["key_file_path"];
        fpgaPara.inputSize = stringToNum<uint32_t>(string_split(mpFpga["input_resolution"], 'x')[0]);
        fpgaPara.inputOffset = mpFpga["input_offest"];
        fpgaPara.outputOffset = mpFpga["output_offest"];
        fpgaPara.outputSize = hexToInt(mpFpga["output_size"]);
        fpgaPara.outputSigType = stringToNum<uint32_t>(mpFpga["output_signal_type"]);
        fpgaPara.outputWaitTime = stringToNum<uint32_t>(mpFpga["output_wait_time"]);
        fpgaPara.inputReg = mpFpga["input_reg"];
        fpgaPara.outputReg = mpFpga["output_reg"];
        fpgaPara.interuptDev = mpFpga["fpga_interupt_dev"];

        picPara.processMode = stringToNum<int>(mpPic["process_mode"]);
        picPara.inputScale = stringToNum<double>(mpPic["input_scale"]);
        picPara.inputZero = stringToNum<float>(mpPic["input_zero"]);
        picPara.rMean = stringToNum<float>(mpPic["r_mean"]);
        picPara.gMean = stringToNum<float>(mpPic["g_mean"]);
        picPara.bMean = stringToNum<float>(mpPic["b_mean"]);
        picPara.rStd = stringToNum<float>(mpPic["r_std"]);
        picPara.gStd = stringToNum<float>(mpPic["g_std"]);
        picPara.bStd = stringToNum<float>(mpPic["b_std"]);

        basicPara.saveBox = stringToNum<int>(mpBasic["save_box"]) == 1 ? true : false;
        basicPara.savePic = stringToNum<int>(mpBasic["save_pic"]) == 1 ? true : false;
        basicPara.picDir = mpBasic["pic_path"];

        detectPara.netFile = mpDetect["model_cfg_file"];
        detectPara.labelPath = mpDetect["label_path"];
        detectPara.classFile = mpDetect["class_file"];

        if (containNet)
        {
                netPara.cameraIP = mpCamera["ip"];
                netPara.cameraPort = stringToNum<int>(mpCamera["port"]);
                netPara.cameraUser = mpCamera["user"].substr(1, mpCamera["user"].size() - 2);
                netPara.cameraPassword = mpCamera["password"].substr(1, mpCamera["password"].size() - 2);
                netPara.url = mpCamera["url"].substr(1, mpCamera["url"].size() - 2);
                netPara.rtpSendIP = mpRtp["ip"];
                netPara.rtpSendPort = stringToNum<int>(mpRtp["port"]);
                netPara.boxSendIP = mpBox["ip"];
                netPara.boxSendPort = stringToNum<int>(mpBox["port"]);
        }

        mpBasic.clear();
        mpFpga.clear();
        mpPic.clear();
        mpDetect.clear();
        mpCamera.clear();
        mpRtp.clear();
        mpBox.clear();

        return fpgaPara.isValid();
}

const PicParaCfg &Common::getPicCfg()
{
        return picPara;
}

const BasicParaCfg &Common::getBasicPara()
{
        return basicPara;
}

const FpgaParaCfg &Common::getFpgaPara()
{
        return fpgaPara;
}

const DetectParaCfg &Common::getDetectPara()
{
        return detectPara;
}

const NetParaCfg &Common::getNetPara()
{
        return netPara;
}

bool Common::isSavePicBox()
{
        return basicPara.saveBox;
}
