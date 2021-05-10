//
// Created by ldd on 19-10-22.
//

#ifndef FSERVO_FSERVO_H
#define FSERVO_FSERVO_H

#include <cstdint>
#include <functional>
#include <map>
#include <string>

#include "typesdef.h"

using namespace snowlake;

#define MAX_SUPPORT_SIZE 608

//DCU init status
typedef enum
{
        //DCU未初始化
        DCU_INIT_NOT_INIT = 0,
        //DCU初始化成功
        DCU_INIT_SUCCESS,
        //DCU初始化失败
        DCU_INIT_FAILED
} dcu_init_status_en;

//DCU status, include DCU output number, DCU remain number, DCU init status
typedef struct
{
        //DCU已经计算了多少帧
        uint64_t outputCount;
        //剩余多少帧未计算
        uint64_t remain_number;
        //DCU初始化状态
        dcu_init_status_en status;
} dcu_status_st;

typedef struct
{
        //fpga版本号
        std::string dcu_fpga_version;
        //fservo版本号
        std::string dcu_fservo_version;
} dcu_version_st;

//FPGA支持的图片分辨率
typedef enum
{
        //608*608
        SIZE_608 = 0,
        //416*416
        SIZE_416 = 5,
        //320*320
        SIZE_320,
        SIZE_352
} picture_resolution_en;

typedef struct
{
        //输入数据(rgba)
        unsigned char *data;
        //输入数据长度
        unsigned int data_len;
        //输入数据的尺寸
        picture_resolution_en pic_resolution;
        //图片外部唯一标识符，即id
        uint64_t id;
} dcu_input_st;

enum InputType
{
        FPGA_DECODE,
        ARM_DECODE,
};

typedef struct
{
        //输入数据(RGBA格式)
        unsigned char *in_data;
        //输入数据长度
        unsigned int in_data_len;
        //输入fpga图片尺寸
        //picture_resolution_en picture_resolution;
        //图片标识,和输出对应,用户自己使用
        //unsigned long long index;
        //图片标识,和输出对应,用户自己使用
        //unsigned long long interior_index;
        //原始图片的尺寸
        uint32_t src_h, src_w;
        // InputType input_type;
        // unsigned char *head_data;
        // unsigned char *qt_data;
        // unsigned char *huff_data;
        // unsigned char *encode_data;
        // unsigned int encode_len;
        float ratio_h;
        float ratio_w;
        std::string picName;
        uint32_t pic_id;
        uint64_t timestamp;
} FPGAInput;

typedef struct
{
        float x, y, w, h;
} BoxInfo;

typedef struct
{
        BoxInfo bbox;
        int classes;      // 类别个数
        float *prob;      // score
        float *mask;      // 缩略图
        float objectness; //置信度
        int sort_class;   // 当前框的类别
} DetectionInfo;

typedef struct
{
        //输出结果
        uint8 *out_data;
        int32 out_data_len;
} FPGAOutput;

/**
 * dcu输出回调函数,并且回调所消耗的时间,包含在dcu中fpga输出结果的后期处理中
 * */
// typedef void (*callback_func)(const FPGAInput *, const FPGAOutput *);
typedef std::function<void(const FPGAInput *, const FPGAOutput *)> callback_func;

typedef enum
{
        ERR_ZERO = 0,

        //无效路径
        ERR_INVALID_PATH,

        //缓冲区满
        ERR_BUFF_FULL

} ErrorCode;

#endif //FSERVO_FSERVO_H
