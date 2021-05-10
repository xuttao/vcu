#include "common.h"
#include "config.h"
#include "dataControl.h"
#include "dataModel.h"
#include "dataReceive.h"
#include "dataSend.h"
#include "fileManager.h"
#include "fpgaControl.h"
#include "fpgaDecode.h"
#include "fservo.h"
#include "libyuv.h"
#include "log.h"
#include "paramManager.h"
#include "picBufferManager.hpp"
#include "picProcess.h"
#include "rtpDataManager.h"
#include "rtspDataManager.h"
#include "serialPortDataManager.h"
#include "test/myTest.hpp"
#include "turbojpeg.h"
#include "vcuControl.h"
#include "vcuModel.h"
#include "yoloDetect.h"
#include <atomic>
#include <fstream>
#include <functional>
#include <map>
#include <opencv2/opencv.hpp>
#include <queue>
#include <semaphore.h>
#include <stdlib.h>
#ifdef _Unix
#include <unistd.h>
#endif

using namespace snowlake;

#define VCU
#define YOLO
#define SENDBOX

namespace
{
        uint8 g_r_table[256], g_g_table[256], g_b_table[256];
        char **names_fptr = nullptr;

        std::vector<cv::String> pic_files;
        std::vector<cv::String> vecRes;

        typedef std::function<void(const FPGAInput *, const FPGAOutput *)> Func;

        std::string out_pic_path;

        std::atomic<int> pic_num = {0};
        static int pic_totNum = 0;
        struct PicParam
        {
                size_t begin_pos;
                size_t end_pos;
                PicParam(size_t first, size_t last) : begin_pos(first), end_pos(last) {}
        };

        static const int BoxInfoLen = sizeof(FpgaBoxInfo);

        static bool thresh_flag = true;

        FPGAControl *pFpgaControl = nullptr;

        DataReceive *pReceiveControl = nullptr;

        RtpDataManager *pRtpManager = nullptr;

        uint32_t detect_w, detect_h;

        bool write_img_file_flag = false;

        bool save_bbox_flag = true;

        InputBufferManager *pBufferManager = nullptr;

        std::ofstream h264_stream;
        std::ofstream h265_stream;

        struct RgbData2
        {
                uint8_t *pData = nullptr;
                int len = 0;
                uint16_t src_h = 0;
                uint16_t src_w = 0;
                uint16_t scale_h = 0;
                uint16_t scale_w = 0;

                RgbData2() = default;
                RgbData2(uint8_t *_data, int _len) : pData(_data), len(_len)
                {
                }

                RgbData2(uint16_t h, uint16_t w) : src_h(h), src_w(w) {}
        };

        struct YuvModel
        {
                uint8_t *pData = nullptr;
                uint32_t len = 0;
                uint16_t src_h = 0;
                uint16_t src_w = 0;
                uint32_t seq = 0;

                YuvModel(uint8_t *_data, uint32_t _len, uint16_t _h, uint16_t _w, uint32_t _seq) : src_h(_h), src_w(_w), len(_len), seq(_seq)
                {
                        pData = new uint8_t[_len];
                        memcpy(pData, _data, _len);
                }
                YuvModel() = default;
        };

        std::queue<YuvModel> que_yuv;
        std::mutex mutex_yuv;

        sem_t sem_vcu;
        std::mutex mutex_que;
        std::queue<RgbData2> que_rgb;

        // std::atomic_uint8_t times = {0};
        std::atomic_uint32_t pic_count = {0};

} // namespace

namespace
{
        std::queue<FPGAInput *> pic_queue;
        std::mutex pic_mutex;

        std::queue<uint32_t> que_timestamp;
        std::mutex mutex_timestamp;
        bool isfirst = true;
        CODEC_TYPE codecType;
} // namespace

char **getClassName()
{
        return FileManager::getInstance()->parserClassFile(Common::getDetectPara().classFile.c_str());
}

uint8_t *resize_data(unsigned char *pdata, int size, int height, int width, int dst_height, int dst_width)
{
        cv::Mat inputData(height, width, CV_8UC3, cv::Scalar(0, 0, 0));
        // memcpy(inputData.data, pdata, size);
        inputData.data = pdata;

        float ratio1 = (float)dst_height / (float)height;
        float ratio2 = (float)dst_width / (float)width;
        float ratio = ratio1 < ratio2 ? ratio1 : ratio2;

        int real_height = ratio * height;
        int real_width = ratio * width;

        cv::Mat resData(real_height, real_width, CV_8UC3, cv::Scalar(0, 0, 0));
        cv::resize(inputData, resData, cv::Size(real_width, real_height), cv::InterpolationFlags::INTER_LINEAR);

        int begin_h = (dst_height - real_height) / 2;
        int begin_w = (dst_width - real_width) / 2;
        int end_h = real_height + begin_h;
        int end_w = real_width + begin_w;

        unsigned char *pRes = new unsigned char[dst_height * dst_width * 4];
        memset(pRes, 0, dst_height * dst_width * 4);
        unsigned char *pRgb = (unsigned char *)resData.data;
        for (int i = begin_h; i < end_h; i++)
        {
                for (int j = begin_w; j < end_w; j++)
                {
                        int pos = (i * dst_width + j) * 4;
                        int pos2 = (i - begin_h) * resData.step + (j - begin_w) * resData.channels();

                        // pRes[pos] = pRgb[pos2 + 2];
                        // pRes[pos + 1] = pRgb[pos2 + 1];
                        // pRes[pos + 2] = pRgb[pos2];-+

                        memcpy(pRes + pos, pRgb + pos2, 3);

                        // pRes[pos] = pRgb[pos2];
                        // pRes[pos + 1] = pRgb[pos2 + 1];
                        // pRes[pos + 2] = pRgb[pos2 + 2];
                }
        }
        resData.release();
        return pRes;
}

uint8_t *pad_data(const RgbData2 &RGBdata)
{
        int begin_h = (detect_h - RGBdata.scale_h) / 2;
        int begin_w = (detect_w - RGBdata.scale_w) / 2;
        int end_h = RGBdata.scale_h + begin_h;
        int end_w = RGBdata.scale_w + begin_w;

        unsigned char *pRes = new unsigned char[detect_w * detect_h * 4];
        // unsigned char *pRes = pBufferManager->getBufferToSavePicMt();

        memset(pRes, 0, detect_w * detect_h * 4);

        const uint8_t *pRgb = RGBdata.pData;
        for (int i = begin_h; i < end_h; i++)
        {
                for (int j = begin_w; j < end_w; j++)
                {
                        int pos = (i * detect_w + j) * 4;
                        int pos2 = (i - begin_h) * detect_w * 3 + (j - begin_w) * 3;

                        memcpy(pRes + pos, pRgb + pos2, 3);

                        // pRes[pos] = pRgb[pos2 + 2];
                        // pRes[pos + 1] = pRgb[pos2 + 1];
                        // pRes[pos + 2] = pRgb[pos2];
                }
        }

        return pRes;
}

void yuv_to_rgb(const uint8_t *pYUV, int len, uint16_t src_height, uint16_t src_width, RgbData2 &rgbRes)
{
        const static int y_src_pos = src_height * src_width;
        const static int u_src_pos = (src_width * src_height * 5) >> 2;

        const uint8_t *ySrc = pYUV;
        const uint8_t *uSrc = pYUV + y_src_pos;
        const uint8_t *vSrc = pYUV + u_src_pos;

        rgbRes.len = src_height * src_width * 3;
        rgbRes.pData = new uint8_t[rgbRes.len];
        // rgbRes.scale_h = real_height;
        // rgbRes.scale_w = real_width;

        uint8_t *pRGB = rgbRes.pData;
        libyuv::I420ToRGB24(ySrc, src_width, uSrc, src_width >> 1, vSrc, src_width >> 1, pRGB, src_width * 3, src_width, src_height);
}

void yuv_to_rgb_scale(const uint8_t *pYUV, int len, uint16_t src_height, uint16_t src_width, RgbData2 &rgbRes)
{
        const static float ratio1 = (float)detect_h / (float)src_height;
        const static float ratio2 = (float)detect_w / (float)src_width;
        const static float ratio = ratio1 < ratio2 ? ratio1 : ratio2;

        const static int real_height = ratio * src_height + 0.5;
        const static int real_width = ratio * src_width + 0.5;

        // const static int real_height = detect_h;
        // const static int real_width = detect_w;

        const static int y_src_pos = src_height * src_width;
        const static int u_src_pos = (src_width * src_height * 5) >> 2;
        // const static int u_src_pos = (src_width >> 1) * (src_height >> 1);

        const uint8_t *ySrc = pYUV;
        const uint8_t *uSrc = pYUV + y_src_pos;
        const uint8_t *vSrc = pYUV + u_src_pos;
        // const uint8_t *vSrc = pYUV + y_src_pos + u_src_pos;

        const static int y_dst_pos = real_height * real_width;
        const static int u_dst_pos = (real_height * real_width * 5) >> 2;
        // const static int u_dst_pos = (real_height >> 1) * (real_width >> 1);

        uint8_t *pDst = new uint8_t[len];

        uint8_t *yDst = pDst;
        uint8_t *uDst = pDst + y_dst_pos;
        uint8_t *vDst = pDst + u_dst_pos;
        // uint8_t *vDst = pDst + y_dst_pos + u_dst_pos;

        libyuv::I420Scale(ySrc, src_width, uSrc, src_width >> 1, vSrc, src_width >> 1,
                          src_width, src_height,
                          yDst, real_width, uDst, real_width >> 1, vDst, real_width >> 1,
                          real_width, real_height,
                          libyuv::FilterMode::kFilterLinear);

        rgbRes.len = real_height * real_width * 3;
        rgbRes.pData = new uint8_t[rgbRes.len];
        rgbRes.scale_h = real_height;
        rgbRes.scale_w = real_width;

        uint8_t *pRGB = rgbRes.pData;
        libyuv::I420ToRGB24(yDst, real_width, uDst, real_width >> 1, vDst, real_width >> 1, pRGB, real_width * 3, real_width, real_height);

        delete[] pDst;
}

void generate_rgb_table()
{
        const PicParaCfg &picCfg = Common::getPicCfg();
        float tmp;
        LOG_INFO("Preocess Mode:%d", picCfg.processMode == 1);
        if (picCfg.processMode == 1)
        { //在线
                LOG_INFO("online:rMean:%f gMean:%f bMean:%f rStd:%f gStd:%f bStd:%f inputZero:%f inputScale:%f",
                         picCfg.rMean, picCfg.gMean, picCfg.bMean, picCfg.rStd, picCfg.gStd, picCfg.bStd, picCfg.inputZero, picCfg.inputScale);
                for (int i = 0; i < 256; i++)
                {
                        tmp = (i / 255.0 - picCfg.rMean) / picCfg.rStd * 127 / picCfg.inputScale + picCfg.inputZero + 0.5;
                        tmp = tmp < 0 ? 0 : tmp;
                        tmp = tmp > 255 ? 255 : tmp;
                        g_r_table[i] = tmp > 255 ? 255 : (tmp < 0 ? 0 : tmp);

                        tmp = (i / 255.0 - picCfg.gMean) / picCfg.gStd * 127 / picCfg.inputScale + picCfg.inputZero + 0.5;
                        tmp = tmp < 0 ? 0 : tmp;
                        tmp = tmp > 255 ? 255 : tmp;
                        g_g_table[i] = tmp > 255 ? 255 : (tmp < 0 ? 0 : tmp);

                        tmp = (i / 255.0 - picCfg.bMean) / picCfg.bStd * 127 / picCfg.inputScale + picCfg.inputZero + 0.5;
                        tmp = tmp < 0 ? 0 : tmp;
                        tmp = tmp > 255 ? 255 : tmp;
                        g_b_table[i] = tmp > 255 ? 255 : (tmp < 0 ? 0 : tmp);
                }
        }
        else
        { //离线
                LOG_INFO("offline:rMean:%f gMean:%f bMean:%f rStd:%f gStd:%f bStd:%f inputZero:%f inputScale:%f",
                         picCfg.rMean, picCfg.gMean, picCfg.bMean, picCfg.rStd, picCfg.gStd, picCfg.bStd, picCfg.inputZero, picCfg.inputScale);
                for (int i = 0; i < 256; i++)
                {
                        tmp = (i / 1.0 - picCfg.rMean) / picCfg.rStd / picCfg.inputScale + picCfg.inputZero + 0.5;
                        tmp = tmp < 0 ? 0 : tmp;
                        tmp = tmp > 255 ? 255 : tmp;
                        g_r_table[i] = tmp > 255 ? 255 : (tmp < 0 ? 0 : tmp);

                        tmp = (i / 1.0 - picCfg.gMean) / picCfg.gStd / picCfg.inputScale + picCfg.inputZero + 0.5;
                        tmp = tmp < 0 ? 0 : tmp;
                        tmp = tmp > 255 ? 255 : tmp;
                        g_g_table[i] = tmp > 255 ? 255 : (tmp < 0 ? 0 : tmp);

                        tmp = (i / 1.0 - picCfg.bMean) / picCfg.bStd / picCfg.inputScale + picCfg.inputZero + 0.5;
                        tmp = tmp < 0 ? 0 : tmp;
                        tmp = tmp > 255 ? 255 : tmp;
                        g_b_table[i] = tmp > 255 ? 255 : (tmp < 0 ? 0 : tmp);
                }
        }
}

void getPicFromDir(const char *_path)
{
        LOG_INFO("pic path:%s", _path);
        std::vector<std::string> &vecPic = FileManager::getInstance()->getDetPicFile(_path);

        pic_files.reserve(vecPic.size());
        for (int i = 0; i < vecPic.size(); i++)
        {
                pic_files.push_back(vecPic[i].c_str());
        }
        //std::sort(pic_files.begin(),pic_files.end());
}

void drawPicBox(cv::Mat &imgdata, void *_pBox, int boxNum)
{
        //int boxNum=len/BoxInfoLen;
        FpgaBoxInfo *pBox = (FpgaBoxInfo *)_pBox;
        for (int num = 0; num < boxNum; num++)
        {
                FpgaBoxInfo *pBoxData = pBox + num;

                int x1 = pBoxData->x1;
                int y1 = pBoxData->y1;
                int x2 = pBoxData->x2;
                int y2 = pBoxData->y2;

                x1 = (x1 < 0) ? 0 : x1;
                x1 = (x1 > imgdata.cols - 1) ? imgdata.cols - 1 : x1;
                y1 = (y1 < 0) ? 0 : y1;
                y1 = (y1 > imgdata.rows - 1) ? imgdata.rows - 1 : y1;

                x2 = (x2 < 0) ? 0 : x2;
                x2 = (x2 > imgdata.cols - 1) ? imgdata.cols - 1 : x2;
                y2 = (y2 < 0) ? 0 : y2;
                y2 = (y2 > imgdata.rows - 1) ? imgdata.rows - 1 : y2;

                float score = pBoxData->score;

                cv::Rect r(x1, y1, x2 - x1, y2 - y1);
                cv::rectangle(imgdata, r, cv::Scalar(0, 255, 203), 1);

                std::cout << "---------------draw box :" << x1 << " " << y1 << " " << x2 - x1 << " " << y2 - y1 << " " << std::endl;

                int fontFace = CV_FONT_HERSHEY_SIMPLEX;
                double fontScale = 0.4; //字体缩放比
                int thickness = 1;
                int baseline = 0;

                cv::Size textSize = cv::getTextSize(pBoxData->label + std::to_string(score), fontFace, fontScale, thickness, &baseline);
                cv::rectangle(imgdata, cv::Point(x1, y1), cv::Point(x1 + textSize.width + thickness, y1 - textSize.height - 4), cv::Scalar(0, 255, 203), -1);
                cv::Point txt_org(x1, y1 - 4);
                cv::putText(imgdata, pBoxData->label + " " + std::to_string(score), txt_org, fontFace, fontScale, cv::Scalar(0, 0, 0));
        }
}

void drawPicBox2(cv::Mat &imgdata, void *_pBox, int boxNum)
{
        //int boxNum=len/BoxInfoLen;
        BoxInfo2 *pBox = (BoxInfo2 *)_pBox;
        for (int num = 0; num < boxNum; num++)
        {
                BoxInfo2 *pBoxData = pBox + num;

                int x1 = pBoxData->left;
                int y1 = pBoxData->top;
                int x2 = pBoxData->right;
                int y2 = pBoxData->bottom;

                float score = pBoxData->score;

                cv::Rect r(x1, y1, x2 - x1, y2 - y1);
                cv::rectangle(imgdata, r, cv::Scalar(0, 255, 203), 2, 1, 0);

                // std::cout << "---------------draw box :" << x1 << " " << y1 << " " << x2 - x1 << " " << y2 - y1 << " " << std::endl;

                int fontFace = CV_FONT_HERSHEY_SIMPLEX;
                double fontScale = 0.4; //字体缩放比
                int thickness = 1;
                int baseline = 0;

                cv::Size textSize = cv::getTextSize(std::string(names_fptr[pBoxData->classNo]) + std::to_string(score), fontFace, fontScale, thickness, &baseline);
                cv::rectangle(imgdata, cv::Point(x1 - 1, y1), cv::Point(x1 + textSize.width + thickness, y1 - textSize.height - 4), cv::Scalar(0, 255, 203), -1, 1, 0);
                cv::Point txt_org(x1 - 1, y1 - 4);
                cv::putText(imgdata, std::string(names_fptr[pBoxData->classNo]) + " " + std::to_string(score), txt_org, fontFace, fontScale, cv::Scalar(0, 0, 0));
        }
}

void fpga_callback(const FPGAInput *fpgaInput_ptr, const FPGAOutput *fpgaOutput_ptr)
{
        // LOG_INFO("---------call back");
        // delete[] fpgaInput_ptr->in_data;
        // return;

        static NetDetect netDetect(Common::getDetectPara().netFile.c_str(), detect_w, detect_h);
        int num = 0;
        BoxInfo2 *pBox = netDetect.getDetectBoxInfo((char *)fpgaOutput_ptr->out_data, fpgaInput_ptr->src_w, fpgaInput_ptr->src_h, num);
#if 0
        {
                const float scale_h = fpgaInput_ptr->src_h / (float)detect_h;
                const float scale_w = fpgaInput_ptr->src_w / (float)detect_w;

                int boxNum = num;
                for (int num = 0; num < boxNum; num++)
                {

                        BoxInfo2 *pBoxData = pBox + num;

                        pBoxData->left = pBoxData->left * scale_w;
                        pBoxData->top = pBoxData->top * scale_h;
                        pBoxData->right = pBoxData->right * scale_w;
                        pBoxData->bottom = pBoxData->bottom * scale_h;
                }
        }
#endif
        static auto t1 = get_current_Mtime();
        static int times = 0;
        times++;
        if (times % 30 == 0)
        {
                auto t2 = get_current_Mtime();
                std::cout << "-------------------fps: " << 30 / (float)(t2 - t1) * 1000 << " box num: " << num << std::endl;
                fflush(stdout);
                t1 = t2;
        }

#ifdef SENDBOX

        DataSend::getInstance()->sendBox(pBox, num, fpgaInput_ptr->pic_id, fpgaInput_ptr->timestamp);

#endif

        // pBufferManager->releaseBufferMt();

#if 0
        std::string fileTemp;
        for (int i = 0; i < num; i++)
        {
                BoxInfo2 &box = pBox[i];
                std::ostringstream ostr;

                int left = box.left;
                int right = box.right;
                int top = box.top;
                int bottom = box.bottom;

                int pos1 = fpgaInput_ptr->picName.find_last_of('/');

                fileTemp = fpgaInput_ptr->picName.substr(pos1 + 1, fpgaInput_ptr->picName.size() - pos1 - 1);

                ostr << fileTemp << " " << names_fptr[box.classNo] << " " << box.score << " "
                     << left << " " << top << " " << right << " " << bottom;

                // cv::String ss = std::string(ostr.str()).c_str();

                LOG_INFO("%s", std::string(ostr.str()).c_str());

                vecRes.push_back(ostr.str());
        }
        if (write_img_file_flag && num > 0)
        {
                auto cstr = out_pic_path + "/" + fileTemp;
                // auto cstr = out_pic_path + "/" + fileTemp + ".jpg";
                // LOG_INFO("------%s", cstr.c_str());
                cv::Mat im = cv::imread(fpgaInput_ptr->picName);
                drawPicBox2(im, pBox, num);
                bool res = cv::imwrite(cstr, im);
                if (!res)
                {
                        LOG_ERR("## save file fail : %s\n", cstr.c_str());
                }
        }
#endif
        // pic_num++;
        delete[] fpgaInput_ptr->in_data;
}

void vcu_callback(char *pYUV, int len, const DecodeParam &para)
{
        // static auto t1 = get_current_Mtime();
        // static uint64_t count = 0;
        // count++;
        // if (count % 30 == 0)
        // {
        //         auto t2 = get_current_Mtime();
        //         std::cout << "---------------------vcu fps:" << 30.f / (t2 - t1) * 1000 << std::endl;
        //         t1 = t2;
        // }
        // return;
        // std::cout << "---------------------vcu fps:" << count << std::endl;
#if 0
        static int num = 0;
        static uint32_t pic_index = 0;

        uint8_t *pSrc = (uint8_t *)pYUV;
        RgbData2 RGB_data(para.height, para.width);
        yuv_to_rgb(pSrc, len, para.height, para.width, RGB_data);

        uint8_t *pPad = resizeRgbBin(RGB_data.pData, para.width, para.height, detect_w, detect_h);

        delete[] RGB_data.pData;
        // delete[] pPad;

        FPGAInput *pInput = new FPGAInput;
        pInput->in_data = pPad;
        pInput->in_data_len = detect_w * detect_h * 4;
        pInput->pic_id = ++pic_index;

        pInput->src_h = para.height;
        pInput->src_w = para.width;

        pic_mutex.lock();
        if (pic_queue.size() >= 1)
        {
                FPGAInput *head = pic_queue.front();
                delete[] head->in_data;
                delete head;
                pic_queue.pop();
        }
        pic_queue.push(pInput);
        pic_mutex.unlock();

#endif
        if (!DataSend::getInstance()->canSendBox())
        {
                return;
        }

#if 1
        uint32_t time = 0;
        {
                std::lock_guard<std::mutex> locker(mutex_timestamp);
                // LOG_INFO("-------------size:%d,%d", que_timestamp.size(), que_timestamp.front());
                if (que_timestamp.empty())
                {
                        LOG_ERR("empty timestamp!");
                        assert(false);
                }

                time = que_timestamp.front();
                que_timestamp.pop();
        }

        RgbData2 RGB_data(para.height, para.width);
        yuv_to_rgb_scale((uint8_t *)pYUV, len, para.height, para.width, RGB_data);

        auto *pPad = pad_data(RGB_data);

        FPGAInput *pInput = new FPGAInput;
        pInput->in_data = pPad;
        pInput->in_data_len = detect_w * detect_h * 4;
        pInput->pic_id = time;
        // pInput->timestamp = clock_msec();

        pInput->src_h = para.height;
        pInput->src_w = para.width;

        pic_mutex.lock();
        if (pic_queue.size() > 1)
        {
                FPGAInput *head = pic_queue.front();
                delete[] head->in_data;
                delete head;
                pic_queue.pop();
        }
        pic_queue.push(pInput);
        pic_mutex.unlock();

        delete[] RGB_data.pData;

#endif

#if 0
        mutex_yuv.lock();
        if (que_yuv.size() > 1)
        {
                delete[] que_yuv.front().pData;
                que_yuv.pop();
        }
        que_yuv.push(YuvModel((uint8_t *)pYUV, len, para.height, para.width, ++pic_index));

        mutex_yuv.unlock();

        sem_post(&sem_vcu);

#endif

#if 0

        int width = para.width, height = para.height;

        char *yData = pYUV;
        char *vData = &pYUV[width * height];
        char *uData = &vData[width * height >> 2];

        static int rgb_len = width * height * 3;
        uint8_t *pRgb = new uint8_t[rgb_len];
        yuv420_2_rgb888((uint8_t *)pRgb, (uint8_t *)yData, (uint8_t *)uData, (uint8_t *)vData, width, height, width, width >> 1, width * 3, yuv2rgb565_table, 0);

#if 0
        {
                auto *pp = resize_data(pRgb, 0, 1080, 1920, 416, 416);
                std::cout << "--------------1time:" << get_current_Mtime() - t1 << std::endl;

                FileManager::getInstance()->writeBinaryFile((char *)pp, 416 * 416 * 4, "opencv.bin");

                t1 = get_current_Mtime();
                RgbData2 RGB_data(para.height, para.width);
                yuv_to_rgb_scale((uint8_t *)pYUV, len, para.height, para.width, RGB_data);

                // FileManager::getInstance()->writeBinaryFile((char *)RGB_data.pData, 234 * 416 * 3, "libyuv_before.bin");

                auto *pp2 = pad_data(RGB_data);
                std::cout << "--------------2time:" << get_current_Mtime() - t1 << std::endl;

                FileManager::getInstance()->writeBinaryFile((char *)pp2, 416 * 416 * 4, "libyuv.bin");
                exit(0);
        }
#endif

        pic_index++;

        mutex_que.lock();
        if (que_rgb.size() > 3)
        {
                auto head = que_rgb.front();
                que_rgb.pop();
                delete[] head.pData;
        }
        RgbData2 rgb(pRgb, rgb_len);
        rgb.src_h = height;
        rgb.src_w = width;
        que_rgb.push(rgb);
        mutex_que.unlock();

        sem_post(&sem_vcu);

#endif

#if 0
        // auto t1 = get_current_Mtime();
        RgbData2 RGB_data(para.height, para.width);
        yuv_to_rgb_scale((uint8_t *)pYUV, len, para.height, para.width, RGB_data);

        mutex_que.lock();
        if (que_rgb.size() > 2)
        {
                auto head = que_rgb.front();
                que_rgb.pop();
                delete[] head.pData;
        }
        que_rgb.push(RGB_data);
        mutex_que.unlock();

        sem_post(&sem_vcu);

        // std::cout << "---------------time:" << get_current_Mtime() - t1 << std::endl;

        // FileManager::getInstance()->writeBinaryFile((char *)p, rgb_len, "out1080.bin");
#endif

#if 0
        static int seq = 1;
        char num[40];
        sprintf(num, "out/decoded_rgb_%04d.bin", seq++);

        std::ofstream out(num, std::ofstream::binary);
        out.write(pRgb, 720 * 1280 * 3);
        out.close();

        delete[] pRgb;
#endif
}

void rtp_callback(const char *pdata, int len, bool mark, uint32_t time)
{
        // h264_stream.write(pdata, len);
        // h264_stream.flush();

        // h265_stream.write(pdata, len);
        // h265_stream.flush();
        // LOG_INFO("receive packet len:%d", len);
#ifdef VCU

        VCUControl::getInstance()->pushData(pdata, len, time);
        if (mark)
        {
                if (!isfirst || CODEC_TYPE::H265 == codecType)
                {
                        std::lock_guard<std::mutex> locker(mutex_timestamp);
                        que_timestamp.push(time);

                        // LOG_INFO("------------------------------time:%d", time);
                }
                else
                {
                        isfirst = false;
                }
        }

#else
        LOG_INFO("receive packet len:%d", len);
#endif
}

#if 0
void preprocessing_pic(void *arg)
{
        PicParam *pParam = (PicParam *)arg;

        int begin = pParam->begin_pos;
        int end = pParam->end_pos;

        delete arg;

        FSERVO_LOG(LEVEL_INFO, "pic process id:%u,begin:%d,end:%d", (unsigned int)pthread_self(), begin, end);

        int data_size = 640 * 512 * 3;

        for (size_t i = begin; i < end; i++)
        {
                std::string &&file_path = pic_files[i];

                FPGAInput *pInput = new FPGAInput;
                IplImage *m = cvLoadImage(file_path.c_str(), CV_LOAD_IMAGE_COLOR);

                pInput->in_data = (unsigned char *)malloc(data_size);

                pInput->in_data_len = data_size;

                pInput->picName = file_path;

                // uint64_t source_loc = 0;
                // uint64_t w_st = 0;
                uint8_t *cv_ptr = (uint8_t *)m->imageData;

                // for(int h = 0; h<m->height; ++h){
                //     for(int w = 0; w<m->width; w++){
                //         w_st = (h * m->width + w) * 3;
                //         //r g b
                //         source_loc = h  * m->widthStep + w* m->nChannels;

                //         pInput->in_data[w_st] = g_r_table[cv_ptr[source_loc + 2]];
                //         pInput->in_data[w_st + 1] = g_g_table[cv_ptr[source_loc + 1]];
                //         pInput->in_data[w_st + 2] = g_b_table[cv_ptr[source_loc]];

                //     }
                // }

                uchar *pdes = pInput->in_data;
                for (int h = 0; h < m->height; ++h)
                {
                        uchar *p_pixel = cv_ptr + h * m->widthStep;
                        pdes = pInput->in_data + h * m->widthStep;
                        for (int w = 0; w < m->width; w++)
                        {
                                pdes[0] = g_r_table[p_pixel[2]];
                                pdes[1] = g_g_table[p_pixel[1]];
                                pdes[2] = g_b_table[p_pixel[0]];

                                p_pixel += 3;
                                pdes += 3;
                        }
                }

                pic_mutex.lock();
                pic_queue.push(pInput);
                pic_mutex.unlock();

                cvReleaseImage(&m);
        }
}
void preprocessing_pic3(void *arg)
{
        PicParam *pParam = (PicParam *)arg;
        int begin = pParam->begin_pos;
        int end = pParam->end_pos;

        delete arg;

        LOG_INFO("pic process id:%u,begin:%d,end:%d", std::this_thread::get_id(), begin, end);

        int width = 640;
        int height = 512;
        int data_size = 640 * 512 * 3;

        ifstream inStream;
        tjhandle jpeg_handle = tjInitDecompress();
        int img_buffer_length = 1 * 1024 * 1024;
        uchar *img_buffer = new uchar[img_buffer_length];
        uchar *rgb_buffer = new uchar[width * height * 3];
        int JPEG_QUALITY = 75;

        for (int i = begin; i < end; i++)
        {
                std::string &&file_path = pic_files[i];

                FPGAInput *pInput = new FPGAInput;

                inStream.open(file_path.c_str(), ifstream::binary);
                inStream.seekg(0, inStream.end);
                int length = inStream.tellg();
                inStream.seekg(0, inStream.beg);

                inStream.read((char *)img_buffer, length);
                inStream.close();

                tjDecompressHeader2(jpeg_handle, img_buffer, length, &width, &height, &JPEG_QUALITY);

                //tjscalingfactor scalingFactor = { 1, 1 };
                //width=TJSCALED(width,scalingFactor);
                //height=TJSCALED(height,scalingFactor);
                auto p = pBufferManager->getBufferToSavePicMtTest();
                tjDecompress2(jpeg_handle, img_buffer, length, p, width, 0, height, TJPF_RGB, TJFLAG_FASTUPSAMPLE | TJFLAG_FASTDCT);

                //pInput->in_data=(unsigned char*)malloc(data_size);

                pInput->in_data = p;

                pInput->in_data_len = data_size;

                pInput->picName = file_path;

                pInput->src_h = 512;
                pInput->src_w = 640;

                pic_mutex.lock();
                pic_queue.push(pInput);
                pic_mutex.unlock();
        }

        delete[] img_buffer;
        delete[] rgb_buffer;
        tjDestroy(jpeg_handle);
}
void preprocessing_pic4(void *arg)
{
        PicParam *pParam = (PicParam *)arg;

        int begin = pParam->begin_pos;
        int end = pParam->end_pos;

        delete arg;

        FSERVO_LOG(LEVEL_INFO, "pic process id:%u,begin:%d,end:%d", (unsigned int)pthread_self(), begin, end);

        for (size_t i = begin; i < end; i++)
        {
                std::string &&file_path = pic_files[i];

                IplImage *pSrc_img = cvLoadImage(file_path.c_str(), CV_LOAD_IMAGE_COLOR);

                float scale = std::min<float>(static_cast<float>(detect_w) / (pSrc_img->width * 1.0f), static_cast<float>(detect_h) / (pSrc_img->height * 1.0f));
                CvSize size;
                size.width = static_cast<int>(scale * pSrc_img->width);
                size.height = static_cast<int>(scale * pSrc_img->height);
                IplImage *dst = cvCreateImage(size, pSrc_img->depth, pSrc_img->nChannels);
                cvResize(pSrc_img, dst, CV_INTER_LINEAR);

                int begin_w = (detect_w - size.width) / 2;
                int begin_h = (detect_h - size.height) / 2;
                int end_h = size.height + begin_h;
                int end_w = size.width + begin_w;

                FPGAInput *pInput = new FPGAInput;
                pInput->src_h = pSrc_img->height;
                pInput->src_w = pSrc_img->width;
                pInput->in_data_len = detect_w * detect_h * 4;
                pInput->in_data = new unsigned char[pInput->in_data_len];
                //pInput->in_data = pBufferManager->getBufferToSavePicMt();
                pInput->picName = file_path;

                uint8_t *cv_ptr = (uint8_t *)dst->imageData;

                for (int h = begin_h; h < end_h; ++h)
                {
                        uchar *p_pixel = cv_ptr + (h - begin_h) * dst->widthStep;
                        uchar *pdes = pInput->in_data + h * detect_w * 4;

                        for (int w = begin_w; w < end_w; w++)
                        {
                                int pos1 = (w - begin_w) * dst->nChannels;
                                int pos2 = w * 4;

                                // pdes[pos2] = g_r_table[p_pixel[pos1 + 2]];
                                // pdes[pos2 + 1] = g_g_table[p_pixel[pos1 + 1]];
                                // pdes[pos2 + 2] = g_b_table[p_pixel[pos1]];

                                pdes[pos2] = p_pixel[pos1 + 2];
                                pdes[pos2 + 1] = p_pixel[pos1 + 1];
                                pdes[pos2 + 2] = p_pixel[pos1];
                        }
                }

                // FileManager::getInstance()->writeBinaryFile((char *)pInput->in_data, pInput->in_data_len, "test.bin");

                pic_mutex.lock();
                pic_queue.push(pInput);
                pic_mutex.unlock();

                cvReleaseImage(&pSrc_img);
                cvReleaseImage(&dst);
        }

        LOG_INFO("pic process exit");
}
void preprocessing_pic5(void *arg)
{
        PicParam *pParam = (PicParam *)arg;

        int begin = pParam->begin_pos;
        int end = pParam->end_pos;

        delete arg;

        FSERVO_LOG(LEVEL_INFO, "pic process id:%u,begin:%d,end:%d", (unsigned int)pthread_self(), begin, end);

        for (size_t i = begin; i < end; i++)
        {
                std::string &&file_path = pic_files[i];

                IplImage *pSrc_img = cvLoadImage(file_path.c_str(), CV_LOAD_IMAGE_COLOR);
                // IplImage *pSrc_img = NULL;

                FPGAInput *pInput = new FPGAInput;

                // {
                //         CvSize size;
                //         size.width = 1024;
                //         size.height = 1024;
                //         pSrc_img = cvCreateImage(size, pOri_img->depth, pOri_img->nChannels);
                //         cvResize(pOri_img, pSrc_img, CV_INTER_LINEAR);

                //         float ratio_h = (float)pOri_img->height / 1024.0;
                //         float ratio_w = (float)pOri_img->width / 1024.0;

                //         pInput->ratio_h = ratio_h;
                //         pInput->ratio_w = ratio_w;
                // }

                float scale = std::min<float>(static_cast<float>(detect_w) / (pSrc_img->width * 1.0f), static_cast<float>(detect_h) / (pSrc_img->height * 1.0f));
                CvSize size;
                size.width = static_cast<int>(scale * pSrc_img->width);
                size.height = static_cast<int>(scale * pSrc_img->height);
                IplImage *dst = cvCreateImage(size, pSrc_img->depth, pSrc_img->nChannels);
                cvResize(pSrc_img, dst, CV_INTER_LINEAR);

                int begin_w = (detect_w - size.width) / 2;
                int begin_h = (detect_h - size.height) / 2;
                int end_h = size.height + begin_h;
                int end_w = size.width + begin_w;

                pInput->src_h = pSrc_img->height;
                pInput->src_w = pSrc_img->width;
                pInput->in_data_len = detect_w * detect_h * 4;
                pInput->in_data = new unsigned char[pInput->in_data_len];
                //pInput->in_data = pBufferManager->getBufferToSavePicMt();
                pInput->picName = file_path;

                uint8_t *cv_ptr = (uint8_t *)dst->imageData;

                for (int h = begin_h; h < end_h; ++h)
                {
                        uchar *p_pixel = cv_ptr + (h - begin_h) * dst->widthStep;
                        uchar *pdes = pInput->in_data + h * detect_w * 4;

                        for (int w = begin_w; w < end_w; w++)
                        {
                                int pos1 = (w - begin_w) * dst->nChannels;
                                int pos2 = w * 4;

                                pdes[pos2] = g_r_table[p_pixel[pos1 + 2]];
                                pdes[pos2 + 1] = g_g_table[p_pixel[pos1 + 1]];
                                pdes[pos2 + 2] = g_b_table[p_pixel[pos1]];
                        }
                }

                // FileManager::getInstance()->writeBinaryFile((char *)pInput->in_data, pInput->in_data_len, "test.bin");

                pic_mutex.lock();
                pic_queue.push(pInput);
                pic_mutex.unlock();

                cvReleaseImage(&pSrc_img);
                cvReleaseImage(&dst);
        }

        LOG_INFO("pic process exit");
}
#endif

void input_thread_frompic()
{
        LOG_INFO("input thread start! id:%u", std::this_thread::get_id());
        int size = pic_files.size();
        int times = 0;
        for (int i = 0; i < size;)
        {
                pic_mutex.lock();
                if (pic_queue.empty())
                {
                        pic_mutex.unlock();
#ifdef _Unix
                        usleep(3000);
#else
            std::this_thread::sleep_for(chrono::microseconds(3000);
#endif
                        times++;
                        continue;
                }
                FPGAInput *pInput = pic_queue.front();

                // int len = 0;
                // char *p = readBinaryFile("fpga/fpga_input_1_uint8.bin", len);
                // FSERVO_CHECK(len == 416 * 416 * 4);
                // pInput->in_data = (unsigned char *)p;
                // pInput->in_data_len = 416 * 416 * 4;

                pic_queue.pop();
                pic_mutex.unlock();
                pFpgaControl->input(pInput);

                i++;
        }

        LOG_INFO("--------------------wait queue times:%d", times);

        LOG_INFO("input thread exit");
}

void process_thread_fromvcu()
{
        for (;;)
        {
#if 0
                int ret = sem_wait(&sem_vcu);

                FSERVO_CHECK(ret != -1);

                mutex_que.lock();
                RgbData2 data = que_rgb.front();
                que_rgb.pop();
                mutex_que.unlock();

#if 1
                unsigned char *p_resize_data = resize_data(data.pData, data.len, data.src_h, data.src_w, detect_h, detect_w);
#endif

                FPGAInput *pInput = new FPGAInput;
                pInput->in_data = p_resize_data;
                pInput->in_data_len = detect_h * detect_w * 4;
                pInput->pic_id = 0;

                pInput->src_h = data.src_h;
                pInput->src_w = data.src_w;

                pic_mutex.lock();
                pic_queue.push(pInput);
                pic_mutex.unlock();

                delete[] data.pData;
#else

                int ret = sem_wait(&sem_vcu);

                FSERVO_CHECK(ret != -1);

                mutex_yuv.lock();
                YuvModel Yuvdata = que_yuv.front();
                que_yuv.pop();
                mutex_yuv.unlock();

                RgbData2 RGB_data(Yuvdata.src_h, Yuvdata.src_w);
                yuv_to_rgb_scale(Yuvdata.pData, Yuvdata.len, Yuvdata.src_h, Yuvdata.src_w, RGB_data);

                auto *pPad = pad_data(RGB_data);

                FPGAInput *pInput = new FPGAInput;
                pInput->in_data = pPad;
                pInput->in_data_len = detect_w * detect_h * 4;
                pInput->pic_id = Yuvdata.seq;

                pInput->src_h = Yuvdata.src_h;
                pInput->src_w = Yuvdata.src_w;

                pic_mutex.lock();
                pic_queue.push(pInput);
                pic_mutex.unlock();

                delete[] RGB_data.pData;
                delete[] Yuvdata.pData;
#endif
        }
}

void input_thread_fromvcu()
{
        LOG_INFO("input thread start! id:%u", std::this_thread::get_id());

        for (;;)
        {
                pic_mutex.lock();
                if (pic_queue.empty())
                {
                        pic_mutex.unlock();
#ifdef _Unix
                        usleep(10000);
#else
            std::this_thread::sleep_for(chrono::microseconds(3000);
#endif
                        continue;
                }
                FPGAInput *pInput = pic_queue.front();
                pic_queue.pop();
                pic_mutex.unlock();
                pFpgaControl->input(pInput);
        }

        LOG_INFO("input thread exit");
}

void clear()
{
        LOG_INFO("clear timestamp");
#ifdef VCU
        VCUControl::getInstance()->restart();
#endif
        pic_mutex.lock();
        while (!pic_queue.empty())
        {
                FPGAInput *head = pic_queue.front();
                delete[] head->in_data;
                delete head;
                pic_queue.pop();
        }
        pic_mutex.unlock();

        pic_count = 0;
        isfirst = true;
        while (!que_timestamp.empty())
                que_timestamp.pop();
}

void initCfg()
{
        out_pic_path = get_current_dir() + '/' + "out";
        bool res = FileManager::getInstance()->createDir(out_pic_path);
        if (!res) LOG_ERR("make dir fail:%s", out_pic_path.c_str());

        FSERVO_CHECK(isFileExist("./cfg/fservo.cfg"));
        res = FileManager::getInstance()->parserCfgFile("./cfg/fservo.cfg", CfgType::Basic);
        // ParamManager::getInstance()->parserCfg("./cfg/fservo.cfg");
        // ParamManager::getInstance()->initParam();

        if (!res)
        {
                LOG_ERR("parser fservo.cfg fail!");
                exit(-1);
        }

        bool containNet = false;
        if (isFileExist("cfg/net.cfg"))
        {
                FileManager::getInstance()->parserCfgFile("cfg/net.cfg", CfgType::Net);

                containNet = true;
        }
        Common::initCfg(containNet);

        detect_w = detect_h = Common::getFpgaPara().inputSize;
        LOG_INFO("detect size:%d", detect_w);
        names_fptr = FileManager::getInstance()->parserClassFile(Common::getDetectPara().classFile.c_str());
}

int main(int argc, char **argv)
{
        SerialPortDataManager *p = new SerialPortDataManager;
        SerialPort::PortParam param("/dev/ttyUSB0", 9600, 8, 1);
        p->init(param, NULL);
        p->initParser(SerialPortDataManager::MavLink);
        p->start();
        // sleep(10 * 60);
        // return 0;

        initCfg();
        generate_rgb_table();

        int thread_num = 1;

        write_img_file_flag = Common::getBasicPara().savePic;
        save_bbox_flag = Common::getBasicPara().saveBox;

        LOG_INFO("start");

#ifdef YOLO
        const FpgaParaCfg &fpgaPara = Common::getFpgaPara();
        Func func_call_back = fpga_callback;
        pFpgaControl = new FPGAControl;
        pFpgaControl->initFpga();
        pFpgaControl->setCallBackFun(func_call_back);
        pFpgaControl->start();
#endif

#ifdef SENDBOX
        //登陆

        const std::string &ip = Common::getNetPara().cameraIP;
        int rtsp_port = Common::getNetPara().cameraPort;
        // CODEC_TYPE codecType;
        RtspDataManager::getInstance()->login(ip.c_str(), rtsp_port, codecType);

        if (codecType == CODEC_TYPE::H264)
        {
                LOG_INFO("codec is h264");
        }
        else if (codecType == CODEC_TYPE::H265)
        {
                LOG_INFO("codec is h265");
        }

        //准备接收
        pRtpManager = new RtpDataManager;
        pRtpManager->init("", 9980, rtp_callback, codecType);
        pRtpManager->start();

        DataSend::getInstance()->setpf(clear);

#ifdef VCU

        VCUControl::getInstance()->init(codecType, vcu_callback);
        VCUControl::getInstance()->open();
#endif

        //开始播放
        RtspDataManager::getInstance()->play();

#endif

#ifdef YOLO
#ifndef VCU
        std::string picPath = Common::getBasicPara().picDir;
        getPicFromDir(picPath.c_str());
        pic_totNum = pic_files.size();
        LOG_INFO("pic num is %d", pic_totNum);

        std::thread *threadArr = new std::thread[thread_num];
        for (int i = 0; i < thread_num; i++)
        {
                PicParam *param = new PicParam(0, pic_files.size());
                threadArr[i] = std::thread(preprocessing_pic4, param);
        }

        std::thread input_thread(input_thread_frompic);
        input_thread.join();

        while (pic_totNum != pic_num)
        {
                usleep(10 * 1000);
        }
#else

        std::thread input_thread(input_thread_fromvcu);
        input_thread.join();
#endif
#else

        usleep(10 * 60 * 1000 * 1000);

#endif
        usleep(1000 * 1000);
        LOG_INFO("end");

        if (Common::isSavePicBox())
        {
                std::vector<std::string> vecPos;
                vecPos.reserve(vecRes.size());
                for (int i = 0; i < vecRes.size(); i++)
                {
                        vecPos.push_back(vecRes[i]);
                }
                bool res = FileManager::getInstance()->writeTxtFile(vecPos, "./myBox.txt");
                if (!res)
                        LOG_ERR("save box fail");
                else
                        LOG_INFO("save box success");
        }

        if (pFpgaControl) delete pFpgaControl;
        return 0;
}
