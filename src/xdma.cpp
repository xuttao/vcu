#include "xdma.h"
#include "config.h"
#include "devManager.h"
#include "log.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace snowlake
{

#define REG_MEMORY_BASE (0x20000)

#define MAP_SIZE (64 * 1024)

#define INVALID_FILE_ID -1

#define DEV_CNT_TO_WRITE_DATA 1

#define DEV_CNT_TO_WRITE_WEIGHT 1

#define XDMA_CHANNEL_CNT 2

#define DEV_MEM_ADDR_BASE_0 0x0
#define DEV_MEM_ADDR_BASE_1 0x400000000
#define DEV_MEM_ADDR_BASE_2 0x800000000
#define DEV_MEM_ADDR_BASE_3 0xc00000000

#define FPGA_DOWN 0

#define FPGA_START 1

#define FPGA_CALC_START()                      \
        do                                     \
        {                                      \
                xdma_write_reg(0, FPGA_DOWN);  \
                usleep(1);                     \
                xdma_write_reg(0, FPGA_START); \
        } while (false)

#define FPGA_CALC_START_INDEX(reg_index)               \
        do                                             \
        {                                              \
                xdma_write_reg(reg_index, FPGA_DOWN);  \
                usleep(1);                             \
                xdma_write_reg(reg_index, FPGA_START); \
        } while (false)

        //Read from FPGA devices
        static const char *xdma_list_c2h[XDMA_CHANNEL_CNT] = {
            "/dev/xdma0_c2h_0",
            "/dev/xdma0_c2h_1",
        };

        //Write to FPGA devices
        static const char *xdma_list_h2c[XDMA_CHANNEL_CNT] = {
            "/dev/xdma0_h2c_0",
            "/dev/xdma0_h2c_1",
        };

        static int32 dma_reg_fd = INVALID_FILE_ID;
        static uint32 *dma_reg_base = nullptr;

        static int32 dma_read_fds[XDMA_CHANNEL_CNT] = {INVALID_FILE_ID};
        static int32 dma_write_fds[XDMA_CHANNEL_CNT] = {INVALID_FILE_ID};

        static uint64_t dma_channels_base_addr[XDMA_CHANNEL_CNT] = {
            DEV_MEM_ADDR_BASE_0,
            DEV_MEM_ADDR_BASE_1,
        };

        static boolean is_xdma_dev_opened = false;

        int32 xdma_open(const dev_init_param_st &param)
        {

                int32 ret = ERR_NONE;

                do
                {
                        FSERVO_CHECK_BREAK(false == is_xdma_dev_opened, ret);

                        //Open registers to access FPGA
                        dma_reg_fd = open(param.reg_dev_name, O_RDWR | O_SYNC);
                        if (INVALID_FILE_ID == dma_reg_fd)
                        {
                                FSERVO_LOG(LEVEL_INFO, "DMA_InitReg:unable to open device:[%s].\n", param.reg_dev_name);
                                ret = ERR_NORM;
                                break;
                        }

                        dma_reg_base = (uint32 *)mmap((void *)0,
                                                      MAP_SIZE,
                                                      PROT_READ | PROT_WRITE,
                                                      MAP_SHARED,
                                                      dma_reg_fd,
                                                      REG_MEMORY_BASE);
                        if (nullptr == dma_reg_base)
                        {
                                FSERVO_LOG(LEVEL_INFO, "DMA_InitReg:unable to map memory");
                                ret = ERR_NORM;
                                break;
                        }

                        //Open files to access XDMA
                        for (int i = 0; i < XDMA_CHANNEL_CNT; ++i)
                        {
                                dma_write_fds[i] = open(xdma_list_h2c[i], O_RDWR | O_NONBLOCK);
                                if (INVALID_FILE_ID == dma_write_fds[i])
                                {
                                        FSERVO_LOG(LEVEL_INFO, "DMA_InitDma:unable to open device:[%s].", xdma_list_h2c[i]);
                                        ret = ERR_NORM;
                                        break;
                                }
                        }

                        for (int i = 0; i < XDMA_CHANNEL_CNT; ++i)
                        {
                                dma_read_fds[i] = open(xdma_list_c2h[i], O_RDWR | O_NONBLOCK);
                                if (INVALID_FILE_ID == dma_read_fds[i])
                                {
                                        FSERVO_LOG(LEVEL_INFO, "DMA_InitDma:unable to open device:[%s].", xdma_list_c2h[i]);
                                        close(dma_write_fds[i]);
                                        dma_write_fds[i] = INVALID_FILE_ID;
                                        ret = ERR_NORM;
                                        break;
                                }
                        }

                        is_xdma_dev_opened = true;

                } while (false);

                return ret;
        }

        int32 xdma_close(void)
        {

                FSERVO_CHECK(true == is_xdma_dev_opened);

                //Close register opened for FPGA
                munmap((void *)dma_reg_base, MAP_SIZE);
                if (INVALID_FILE_ID != dma_reg_fd)
                {
                        close(dma_reg_fd);
                        dma_reg_fd = INVALID_FILE_ID;
                }
                dma_reg_base = nullptr;

                //Close files opened for XDMA
                for (int i = 0; i < XDMA_CHANNEL_CNT; ++i)
                {
                        if (INVALID_FILE_ID != dma_read_fds[i])
                        {
                                close(dma_read_fds[i]);
                                dma_read_fds[i] = INVALID_FILE_ID;
                        }
                }

                for (int i = 0; i < XDMA_CHANNEL_CNT; ++i)
                {
                        if (INVALID_FILE_ID != dma_write_fds[i])
                        {
                                close(dma_write_fds[i]);
                                dma_write_fds[i] = INVALID_FILE_ID;
                        }
                }

                is_xdma_dev_opened = false;

                return ERR_NONE;
        }

        int32 xdma_write_reg(uint32 reg_index, int32 data)
        {

                int32 ret = ERR_NONE;
                FSERVO_CHECK(true == is_xdma_dev_opened);

                if (nullptr != dma_reg_base)
                {
                        //           FSERVO_LOG(LEVEL_INFO, "write reg[%d]:0x%x", reg_index, data);
                        dma_reg_base[reg_index] = data;
                        ret = ERR_NORM;
                }

                return ret;
        }

        int32 xdma_read_reg(uint32 reg_index, int32 &data_out)
        {

                int32 ret = ERR_NONE;
                FSERVO_CHECK(true == is_xdma_dev_opened);

                do
                {
                        if (nullptr == dma_reg_base)
                        {
                                ret = ERR_NORM;
                                break;
                        }

                        char *ptr = (char *)dma_reg_base;
                        int index_bits = (uint8)ptr[0];
                        int32 index_num = (uint8)ptr[1];
                        int32 index_bytes = (uint8)ptr[2];
                        int32 i, index, shift_bits, mapped_index;

                        //            FSERVO_LOG(LEVEL_INFO, "bits:%d, num:%d, bytes:%d\n", index_bits, index_num, index_bytes);
                        uint32 index_mask = (1 << index_bits) - 1;

                        if (index_num == 0)
                        {
                                ret = ERR_NORM;
                                break;
                        }

                        for (i = 0; i < index_num; i++)
                        {
                                mapped_index = ((i * index_bits) / 32) + 1;
                                shift_bits = (i * index_bits) % 32;

                                if (shift_bits + index_bits > 32)
                                {
                                        index = ((dma_reg_base[mapped_index] >> shift_bits) & ((1 << (32 - shift_bits)) - 1)) |
                                                ((dma_reg_base[mapped_index + 1] & ((1 << (shift_bits + index_bits - 32)) - 1))
                                                 << (32 - shift_bits));
                                }
                                else
                                {
                                        index = (dma_reg_base[mapped_index] >> shift_bits) & index_mask;
                                }

                                if (index == reg_index)
                                {
                                        data_out = dma_reg_base[1 + index_bytes + i];
                                }
                        }

                } while (false);

                return ret;
        }

        int32 xdma_write(void *dst_ptr, void *data_in_ptr, int32 data_size, data_type_en type, bool batch2)
        {

                FSERVO_CHECK(true == is_xdma_dev_opened);

                static int dev_for_weight_cnt = DEV_CNT_TO_WRITE_WEIGHT;
                static int dev_for_data_cnt = DEV_CNT_TO_WRITE_DATA;

                int32 ret = ERR_NORM;
                int dev_cnt = (type == DATA_TYPE_DATA) ? dev_for_data_cnt : dev_for_weight_cnt;

                for (int i = 0; i < dev_cnt; ++i)
                {

                        ret = write_from_buffer((char *)xdma_list_h2c[i],
                                                dma_write_fds[i],
                                                (char *)data_in_ptr,
                                                data_size,
                                                (type == DATA_TYPE_DATA) ? (uint64_t)dst_ptr : ((uint64_t)dst_ptr + dma_channels_base_addr[i]));

                        FSERVO_CHECK(ret == data_size);
                }

                if (DATA_TYPE_DATA == type)
                {
                        FPGA_CALC_START();
                }

                ret = ERR_NONE;

                return ret;
        }

        int32 xdma_write_size(void *dst_ptr, void *data_in_ptr, uint32 data_size, data_type_en type, int picture_resolution, bool batch2)
        {

                FSERVO_CHECK(true == is_xdma_dev_opened);

                static int dev_for_weight_cnt = DEV_CNT_TO_WRITE_WEIGHT;
                static int dev_for_data_cnt = DEV_CNT_TO_WRITE_DATA;

                int32 ret = ERR_NORM;
                int dev_cnt = (type == DATA_TYPE_DATA) ? dev_for_data_cnt : dev_for_weight_cnt;

                for (int i = 0; i < dev_cnt; ++i)
                {

                        ret = write_from_buffer((char *)xdma_list_h2c[i],
                                                dma_write_fds[i],
                                                (char *)data_in_ptr,
                                                data_size,
                                                (type == DATA_TYPE_DATA) ? (uint64_t)dst_ptr : ((uint64_t)dst_ptr + dma_channels_base_addr[i]));

                        FSERVO_CHECK(ret == data_size);
                }

                if (DATA_TYPE_DATA == type)
                {
                        FPGA_CALC_START_INDEX(picture_resolution);
                }

                ret = ERR_NONE;

                return ret;
        }

        int32 xdma_read(void *src_ptr, void *data_out_ptr, int32 data_size, data_type_en type, bool batch2)
        {

                int32 ret = ERR_NORM;

                if (INVALID_FILE_ID != dma_read_fds[0])
                {
                        read_to_buffer((char *)xdma_list_c2h[0], dma_read_fds[0], (char *)data_out_ptr, data_size,
                                       (uint64_t)src_ptr);
                        ret = ERR_NONE;
                }

                return ret;
        }

        int32 xdma_read_from_dev(void *dst_ptr, int dev_id, int32 data_size, data_type_en type)
        {
                int32 ret = ERR_NORM;
                FSERVO_CHECK(false);

                //        FSERVO_CHECK(true == is_xdma_dev_opened);
                //
                //        if (DATA_TYPE_DATA == type || DATA_TYPE_PARAM == type) {
                //
                ////            FSERVO_LOG(LOG_DEBUG, "shm_read_from_dev_id, dev_mem_addr_shm:%lx, dst_ptr:%lx, data_size:%d begin",
                ////                       dev_mem_addr_shm, dst_ptr, data_size);
                //            ssize_t size = utils_dev_read("", dev_id, (char *) ((uint64) dev_mem_addr_shm + (uint64) dst_ptr),
                //                                          data_size, 0);
                ////            FSERVO_LOG(LOG_DEBUG, "shm_read_from_dev_id end");
                //            FSERVO_CHECK(data_size == size);
                //
                //        } else {
                //            ret = ERR_NONE;
                //        }
                //
                //        if (DATA_TYPE_DATA == type) {
                //            FPGA_CALC_START();
                //        }

                return ret;
        }

        int32 xdma_get_mapped_mem_addr(void **mem_addr_pptr, bool batch2)
        {

                FSERVO_CHECK(false);
                *mem_addr_pptr = nullptr;

                return ERR_NONE;
        }
} // end of namespace snowlake
