#ifdef _ARM64

#include "vcuControl.h"
#include "AllegroVcu.h"
#include "log.h"

VCUControl *VCUControl::getInstance()
{
        static VCUControl ins;
        return &ins;
}

void VCUControl::yuvOut(char *_data, int _len, DecodeParam &_para, void *arg)
{
        VCUControl *pThis = (VCUControl *)(arg);
        pThis->pf_callback(_data, _len, _para);
}

void VCUControl::init(CODEC_TYPE _type, pf_vcu_callbak _pf, int _fps)
{
        if (status == OPEN)
        {
                LOG_WARN("vcu is already opend");
                return;
        }
        fps = fps;
        pf_callback = _pf;
        codec_type = _type;
        pAllegroVcu = new AllegroVcu;
        if (_type == CODEC_TYPE::H264)
        {
                pAllegroVcu->init(AL_CODEC_AVC, yuvOut, (void *)this, false, "");
                if (fps != 0) pAllegroVcu->set(fps);
        }
        else if (_type == CODEC_TYPE::H265)
        {
                pAllegroVcu->init(AL_CODEC_HEVC, yuvOut, (void *)this, false, "");
                if (fps != 0) pAllegroVcu->set(fps);
        }
        else
        {
                FSERVO_CHECK(false);
        }
}

void VCUControl::restart()
{
        pAllegroVcu->close();
        delete pAllegroVcu;

        pAllegroVcu = new AllegroVcu;
        if (codec_type == CODEC_TYPE::H264)
        {
                pAllegroVcu->init(AL_CODEC_AVC, yuvOut, (void *)this, false, "");
                if (fps != 0) pAllegroVcu->set(fps);
        }
        else if (codec_type == CODEC_TYPE::H265)
        {
                pAllegroVcu->init(AL_CODEC_HEVC, yuvOut, (void *)this, false, "");
                if (fps != 0) pAllegroVcu->set(fps);
        }
        pAllegroVcu->open();
        status = OPEN;
}

void VCUControl::open()
{
        pAllegroVcu->open();
        status = OPEN;
}

void VCUControl::pushData(const char *_data, int _len, uint32_t _seq)
{
        pAllegroVcu->push(_data, _len);
}

void VCUControl::clear()
{
        pAllegroVcu->clear();
}

void VCUControl::close()
{
        pAllegroVcu->close();
        status = CLOSE;
}

#endif