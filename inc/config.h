#ifndef _FSERVO_CONFIG_H_
#define _FSERVO_CONFIG_H_

#ifdef __cplusplus
namespace snowlake
{
#endif

/* -----------------------------------------------------------------------
**    FPGA related config begin
** ----------------------------------------------------------------------- */
#if defined(OPT_BUILD_ARM64_ENABLE)
#define FPGA_DEV_REG "/dev/uio0"
#else
#define FPGA_DEV_REG "/dev/xdma0_user"
#endif

#define FPGA_DEV_INTERP "/dev/uio2"

#define FPGA_DEV_MEM "/dev/mem"

#define FPGA_OUTPUT1_PATH_FORMAT "./output/output-test_%d-layer16.bin"

#define FPGA_OUTPUT2_PATH_FORMAT "./output/output-test_%d-layer23.bin"

#if defined(OPT_BUILD_ARM64_ENABLE)
#define DEV_NAME_DEFAULT DEV_NAME_SHARE_MEM
#else
#define DEV_NAME_DEFAULT DEV_NAME_XDMA
#endif

//一块中最大画框数
#define ONE_MAX_BOXS 3

#ifdef USER_ZU3
#define FPGA_CTRL_PARAM_OFFSET 0x0

//#define  FPGA_CFG_WEIGHT_OFFSET            0xc300      //zu3 tiny 608
#define FPGA_CFG_WEIGHT_OFFSET 0x35cf0 //zu3 v3 608

//#define  FPGA_CFG_QUAN_PARAM_OFFSET        0x88b600      //zu3 tiny 608
#define FPGA_CFG_QUAN_PARAM_OFFSET 0x3BFAEF0 //zu3 v3 608

#define FPGA_CFG_POOLING_ZERO_OFFSET 0x892980 //zu3 tiny 608

//#define  FPGA_CFG_POOLING_DET_OFFSET       0x8977e0
#define FPGA_CFG_POOLING_DET_OFFSET 0x8B78E0

        //#define  FPGA_CFG_INPUT_OFFSET             0x6000000      //zu3 tiny 608
        //#define  FPGA_CFG_INPUT_OFFSET             0xB000000   //zu3 v3 608

#define FPGA_CFG_INPUT_DATA_SIZE 0x169000

#define FPGA_CFG_IO_DATA_MAX_NUM 8

#define FPGA_OUT_BUF_CNT_MAX (FPGA_CFG_IO_DATA_MAX_NUM)

//#define  FPGA_CFG_OUTPUT_OFFSET            0x4100000      //zu3 tiny 608
//#define  FPGA_CFG_OUTPUT_OFFSET            0xC000000   //zu3 v3 608

//#define  FPGA_CFG_OUTPUT1_DATA_SIZE        0x5f000
#define FPGA_CFG_OUTPUT1_DATA_SIZE 0x00

#define FPGA_CFG_OUTPUT2_DATA_SIZE 0x169000

#define NEW_OUT_BUF_LEN 0x3f4800

        //#define  FPGA_CFG_OUTPUT_SIZE              0x1C3400      //zu3 tiny 608
        //#define  FPGA_CFG_OUTPUT_SIZE              0x767400   //zu3 v3 608

#define FPGA_SHM_MEM_BLOCK_BASE 0x60000000

#define FPGA_REG_MAPPED_SIZE 4096 //(64*1024)

#define FPGA_MEM_MAPPED_SIZE (256 * 1024 * 1024)

//--608--
#define SRC_RES_608_10_XYHW (19 * 20 * 4 * 4 * 3)
#define SRC_RES_608_10_SI (19 * 20 * 2 * 4 * 3)
#define SRC_RES_608_13_XYHW (38 * 38 * 4 * 4 * 3)
#define SRC_RES_608_13_SI (38 * 38 * 2 * 4 * 3)

//--416--
#define SRC_RES_416_10_XYHW (13 * 14 * 4 * 4 * 3)
#define SRC_RES_416_10_SI (13 * 14 * 2 * 4 * 3)
#define SRC_RES_416_13_XYHW (26 * 26 * 4 * 4 * 3)
#define SRC_RES_416_13_SI (26 * 26 * 2 * 4 * 3)

//--320--
#define SRC_RES_320_10_XYHW (10 * 10 * 4 * 4 * 3)
#define SRC_RES_320_10_SI (10 * 10 * 2 * 4 * 3)
#define SRC_RES_320_13_XYHW (20 * 20 * 4 * 4 * 3)
#define SRC_RES_320_13_SI (20 * 20 * 2 * 4 * 3)

//--608真实尺寸--
#define RES_608_10_XYHW (19 * 19 * 4 * 4 * 3)
#define RES_608_10_SI (19 * 19 * 2 * 4 * 3)
#define RES_608_13_XYHW (38 * 38 * 4 * 4 * 3)
#define RES_608_13_SI (38 * 38 * 2 * 4 * 3)

//--416真实尺寸--
#define RES_416_10_XYHW (13 * 13 * 4 * 4 * 3)
#define RES_416_10_SI (13 * 13 * 2 * 4 * 3)
#define RES_416_13_XYHW (26 * 26 * 4 * 4 * 3)
#define RES_416_13_SI (26 * 26 * 2 * 4 * 3)

//--320真实尺寸--
#define RES_320_10_XYHW (10 * 10 * 4 * 4 * 3)
#define RES_320_10_SI (10 * 10 * 2 * 4 * 3)
#define RES_320_13_XYHW (20 * 20 * 4 * 4 * 3)
#define RES_320_13_SI (20 * 20 * 2 * 4 * 3)

//608-10
#define ONE608_XYHW_10_H 19
#define ONE608_XYHW_10_W 19
#define SRC_ONE608_XYHW_10_W 20
//608-13
#define ONE608_XYHW_13_H 38
#define ONE608_XYHW_13_W 38
#define SRC_ONE608_XYHW_13_W 38

//416-10
#define ONE416_XYHW_10_H 13
#define ONE416_XYHW_10_W 13
#define SRC_ONE416_XYHW_10_W 14
//416-13
#define ONE416_XYHW_13_H 26
#define ONE416_XYHW_13_W 26
#define SRC_ONE416_XYHW_13_W 26

//320-10
#define ONE320_XYHW_10_H 10
#define ONE320_XYHW_10_W 10
#define SRC_ONE320_XYHW_10_W 10
//320-13
#define ONE320_XYHW_13_H 20
#define ONE320_XYHW_13_W 20
#define SRC_ONE320_XYHW_13_W 20

#else // zu3end zu7 begin

#define FPGA_CTRL_PARAM_OFFSET 0x0

#define FPGA_CFG_WEIGHT_OFFSET 0x6000000

#define FPGA_CFG_QUAN_PARAM_OFFSET 0x2000000

#define FPGA_CFG_POOLING_ZERO_OFFSET 0x2100000

#define FPGA_CFG_POOLING_DET_OFFSET 0x2200000

//#define  FPGA_CFG_INPUT_OFFSET             0x1000000
//#define  FPGA_CFG_INPUT_OFFSET             0x8F9F980

#define FPGA_CFG_INPUT_DATA_SIZE 0x169000
//#define FPGA_CFG_INPUT_DATA_SIZE 0x100000

#define FPGA_CFG_IO_DATA_MAX_NUM 8

#define FPGA_OUT_BUF_CNT_MAX (FPGA_CFG_IO_DATA_MAX_NUM)

//#define  FPGA_CFG_OUTPUT_OFFSET            0xA000000

//#define  FPGA_CFG_OUTPUT1_DATA_SIZE        0x5f000
#define FPGA_CFG_OUTPUT1_DATA_SIZE 0x00

#define FPGA_CFG_OUTPUT2_DATA_SIZE 0x169000

#define NEW_OUT_BUF_LEN 0x3f4800

//#define  FPGA_CFG_OUTPUT_SIZE              0x767400

#define FPGA_SHM_MEM_BLOCK_BASE 0x60000000
#define FPGA_SHM_MEM_BLOCK_BASE2 0x80000000

#define FPGA_REG_MAPPED_SIZE 4096 //(64*1024)

#define FPGA_MEM_MAPPED_SIZE (512 * 1024 * 1024)

//--608--
//10层第一块 19(h)*20(w)*4(x, y, h, w)*4(sizeof(float))*3(每块最多3框)
#define SRC_RES_608_10_XYHW (19 * 20 * 4 * 4 * 3)
//10层第二块 19(h)*20(w)*2(类型,自信度)*4(sizeof(float))*3(每块最多3框)
#define SRC_RES_608_10_SI (19 * 20 * 2 * 4 * 3)
#define SRC_RES_608_13_XYHW (38 * 40 * 4 * 4 * 3)
#define SRC_RES_608_13_SI (38 * 40 * 2 * 4 * 3)

//--416--
#define SRC_RES_416_10_XYHW (13 * 16 * 4 * 4 * 3)
#define SRC_RES_416_10_SI (13 * 16 * 2 * 4 * 3)
#define SRC_RES_416_13_XYHW (26 * 28 * 4 * 4 * 3)
#define SRC_RES_416_13_SI (26 * 28 * 2 * 4 * 3)

//--320--
#define SRC_RES_320_10_XYHW (10 * 12 * 4 * 4 * 3)
#define SRC_RES_320_10_SI (10 * 12 * 2 * 4 * 3)
#define SRC_RES_320_13_XYHW (20 * 20 * 4 * 4 * 3)
#define SRC_RES_320_13_SI (20 * 20 * 2 * 4 * 3)

//--608真实尺寸--
#define RES_608_10_XYHW (19 * 19 * 4 * 4 * 3)
#define RES_608_10_SI (19 * 19 * 2 * 4 * 3)
#define RES_608_13_XYHW (38 * 38 * 4 * 4 * 3)
#define RES_608_13_SI (38 * 38 * 2 * 4 * 3)

//--416真实尺寸--
#define RES_416_10_XYHW (13 * 13 * 4 * 4 * 3)
#define RES_416_10_SI (13 * 13 * 2 * 4 * 3)
#define RES_416_13_XYHW (26 * 26 * 4 * 4 * 3)
#define RES_416_13_SI (26 * 26 * 2 * 4 * 3)

//--320真实尺寸--
#define RES_320_10_XYHW (10 * 10 * 4 * 4 * 3)
#define RES_320_10_SI (10 * 10 * 2 * 4 * 3)
#define RES_320_13_XYHW (20 * 20 * 4 * 4 * 3)
#define RES_320_13_SI (20 * 20 * 2 * 4 * 3)

//608-10
#define ONE608_XYHW_10_H 19
#define ONE608_XYHW_10_W 19
#define SRC_ONE608_XYHW_10_W 20
//608-13
#define ONE608_XYHW_13_H 38
#define ONE608_XYHW_13_W 38
#define SRC_ONE608_XYHW_13_W 40

//416-10
#define ONE416_XYHW_10_H 13
#define ONE416_XYHW_10_W 13
#define SRC_ONE416_XYHW_10_W 16
//416-13
#define ONE416_XYHW_13_H 26
#define ONE416_XYHW_13_W 26
#define SRC_ONE416_XYHW_13_W 28

//320-10
#define ONE320_XYHW_10_H 10
#define ONE320_XYHW_10_W 10
#define SRC_ONE320_XYHW_10_W 12
//320-13
#define ONE320_XYHW_13_H 20
#define ONE320_XYHW_13_W 20
#define SRC_ONE320_XYHW_13_W 20

#endif //zu7 end

#define SHOW_BUF_LEN (6)

#define SHOW_WINDOW_LEN (3)

#define CPU_SET_MAIN 0
#define CPU_SET_DETECT 1
#define CPU_SET_DATA_IN 2
#define CPU_SET_DATA_OUT 3

#define FPGA_CFG_OUTPUT_CNT_REG_INDEX 10

#define FPGA_OUTPUT_CNT(dev_op_ptr, output_cnt)                           \
        do                                                                \
        {                                                                 \
                int tmp = 0;                                              \
                dev_op_ptr->read_reg(FPGA_CFG_OUTPUT_CNT_REG_INDEX, tmp); \
                output_cnt = (uint32)tmp;                                 \
        } while (false)

#define FILE_PATH_LEN_MAX 257

#define FILE_WRITE_TO_SHARE_MEM_BEGINE_ADDR (0x70000000)
#define FILE_WRITE_TO_SHARE_MEM_END_ADDR (0x78000000)
#define FILE_READ_FROM_SHARE_MEM_BEGIN_ADDR (0x78000000)
#define FILE_READ_FROM_SHARE_MEM_END_ADDR (0x80000000)
#define FILE_WRITE_TO_SHARE_MEM_MAX_SIZE \
        (FILE_WRITE_TO_SHARE_MEM_END_ADDR - FILE_WRITE_TO_SHARE_MEM_BEGINE_ADDR - 1)
#define FILE_READ_FROM_SHARE_MEM_MAX_SIZE \
        (FILE_READ_FROM_SHARE_MEM_END_ADDR - FILE_READ_FROM_SHARE_MEM_BEGIN_ADDR - 1)

#define INPUT_PICTURE_SUFFIX_NAME_JPG ".jpg"
#define INPUT_PICTURE_SUFFIX_NAME_PNG ".png"
#define COPY_CONFIG_FILE_PATH "./copy_config.txt"

#define INPUT_BIN_SUFFIX_NAME ".bin"

//#define  INPUT_PICTURE_SUFFIX_NAME       ".jpg"
#define INPUT_PICTURE_SUFFIX_NAME ".bmp"

#define INPUT_PICTURE_NAME_FORMAT "%s/test_%d%s"

// waiting 10 ms for each pictrue
#define DETECT_SHOW_WAIT_TIME 0

        /* -----------------------------------------------------------------------
**    FPGA related config end
** ----------------------------------------------------------------------- */

        /* -----------------------------------------------------------------------
**    CNN related config begin
** ----------------------------------------------------------------------- */
        //#define  NETWORK_IN_IM_WIDTH                     608
        //#define  NETWORK_IN_IM_HEIGHT                    608

#define NETWORK_CFG_LABELS_PATH "./cfg/labels"

#define NETWORK_CFG_NAMES_PATH "./cfg/coco_classes.txt"

#define NETWORK_CFG_PATH "./cfg/yolov3-tiny.cfg"

#define DNA_INPUT_FILE_PATH "./key"

        //#define  NETWORK_CFG_THRESH                      0.40

        //#define  NETWORK_CFG_NMS_IOU                     0.3

#define NETWORK_OUT1_WIDTH 19
#define NETWORK_OUT1_HEIGHT 19

#define NETWORK_OUT2_WIDTH 38
#define NETWORK_OUT2_HEIGHT 38

#define NETWORK_OUT_CHAN 255

//fpga输出图片通道数(全通道含无效补零位) 单位float
#define FPGA_CHAN_NUM 64
//fpga输出图片标识 1位 (实物概率) 2位(x,y) 2位(w,h) 单位float
#define FPGA_OUT_MAKR 5
//fpga区域输出最大识别物个数
#define EACH_NUM 3

#define DEFAULT_MAX_BOX 90

//10层坐标
#define ZU3_10_XYHW_SIZE 0x43B0
//10层阈值自信度大小
#define ZU3_10_SI_SIZE 0x21D8
//13层坐标
#define ZU3_13_XYHW_SIZE 0x10EC0
//13层阈值自信度大小
#define ZU3_13_SI_SIZE 0x8760

//10层坐标
#define SRC_ZU3_10_XYHW_SIZE (0x4740)
//10层阈值自信度大小
#define SRC_ZU3_10_SI_SIZE (0x23A0)
//13层坐标
#define SRC_ZU3_13_XYHW_SIZE (0x11D00)
//13层阈值自信度大小
#define SRC_ZU3_13_SI_SIZE (0x8EB0)

        //#define  FPGA_CFG_OUTPUT_SIZE                       (SRC_ZU3_10_XYHW_SIZE + SRC_ZU3_10_SI_SIZE + SRC_ZU3_13_XYHW_SIZE + SRC_ZU3_13_SI_SIZE)

        /* -----------------------------------------------------------------------
**    CNN related config end
** ----------------------------------------------------------------------- */

        //
        //#define CAMERA_WIDTH   (416)
        //#define CAMERA_HIGHT   (416)

#define SRC_CAMERA_WIDTH (640)
#define SRC_CAMERA_HIGHT (480)

#define DST_FPGA_IMG_OFFSET \
        ((CAMERA_HIGHT - (576 / (768 / (CAMERA_HIGHT * 1.0f)))) / 2)

#define FRAME_COUNT 4

//最大线程数
#define GLOBAL_THREAD_NUM (4)

#define PIXEL_BYTES (2)
#define RBG_PIXEL_BYTES (3)
#define RBGA_PIXEL_BYTES (4)

        //#define  FREMEBUFFER_SHOW_BUF_LEN          (CAMERA_WIDTH * CAMERA_HIGHT * PIXEL_BYTES)

        //#define CALCULATE_SHOW_MAX_BUF_LEN  (FPGA_CFG_OUTPUT_SIZE > FREMEBUFFER_SHOW_BUF_LEN ? FPGA_CFG_OUTPUT_SIZE : FREMEBUFFER_SHOW_BUF_LEN)

#define CALCULATE_SHOW_MAX_BUF_LEN (NEW_OUT_BUF_LEN)

/* -----------------------------------------------------------------------
**    Show framebuffer config begin
** ----------------------------------------------------------------------- */
#define SCREEN_OFFSET_EVERY_SECOND (200)
#define FRAMEBUFFER_IMG_SHOW_TIME (3 * 1000000) //unit: us
#define FRAMEBUFFER_IMG_STEPS (300)
#define FRAMEBUFFER_IMG_WAIT_ROLL_TIME (FRAMEBUFFER_IMG_SHOW_TIME / FRAMEBUFFER_IMG_STEPS)
//#define FRAMEBUFFER_IMG_EVERY_STEP_SIZE       (CAMERA_WIDTH / FRAMEBUFFER_IMG_STEPS)
//#define FRAMEBUFFER_END_IMAGE_PAUSE           (1)
/* -----------------------------------------------------------------------
**    Show framebuffer config end
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
**    copy bin to /dev/mem config begin
** ----------------------------------------------------------------------- */
#define COPY_FILE_OFFSET (256) //unit: M
#define COPY_FILE_SIZES ((1024 - COPY_FILE_OFFSET) * 1024 * 1024)
//#define  COPY_ONE_FILE_SIZE              (CAMERA_WIDTH * CAMERA_HIGHT *4)  //image: rgba
#define COPY_FILE_MAX (500)
        /* -----------------------------------------------------------------------
**    copy bin to /dev/mem config end
** ----------------------------------------------------------------------- */

        //#define MIN_BUF (COPY_ONE_FILE_SIZE <= FPGA_CFG_OUTPUT_SIZE ? COPY_ONE_FILE_SIZE : FPGA_CFG_OUTPUT_SIZE)

#ifdef __cplusplus
} // end of namespace snowlake
#endif

#endif //_FSERVO_CONFIG_H_
