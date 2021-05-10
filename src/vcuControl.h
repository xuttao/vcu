/*
 * @Author: xtt
 * @Date: 2020-12-29 21:24:33
 * @Description: ...
 * @LastEditTime: 2021-04-09 14:11:17
 */
#pragma once

#ifdef _ARM64

#include "dataModel.h"
#include "vcuModel.h"
namespace std
{
        class thread;
}

class AllegroVcu;

typedef void (*pf_vcu_callbak)(char *, int, const DecodeParam &);

class VCUControl
{
public:
        enum VCU_STATUS
        {
                OPEN,
                CLOSE,
                ERR,
        };

private:
        VCUControl() : status(CLOSE) {}
        ~VCUControl()
        {
                if (pAllegroVcu) delete pAllegroVcu;
        }

public:
        static VCUControl *getInstance();
        void init(CODEC_TYPE _type, pf_vcu_callbak _pf, int fps = 0);
        void open();
        void pushData(const char *, int, uint32_t);
        void close();
        void restart();
        void clear();

private:
        static void yuvOut(char *, int, DecodeParam &, void *);

private:
        int fps = 0;
        CODEC_TYPE codec_type;
        VCU_STATUS status;
        pf_vcu_callbak pf_callback;
        AllegroVcu *pAllegroVcu = nullptr;
};

#endif