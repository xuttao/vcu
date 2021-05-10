#include "common.h"
#include "config.h"
#include "devManager.h"
#include "fservo.h"
#include "log.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define RW_MAX_SIZE 0x7ffff000

ssize_t utils_dev_read(const char *fname, int fd, char *buffer, uint64_t size,
                       uint64_t base)
{
        ssize_t rc;
        uint64_t count = 0;
        char *buf = buffer;
        off_t offset = base;

        while (count < size)
        {
                uint64_t bytes = size - count;

                if (bytes > RW_MAX_SIZE)
                        bytes = RW_MAX_SIZE;

                if (offset)
                {
                        rc = lseek(fd, offset, SEEK_SET);
                        if (rc != offset)
                        {
                                FSERVO_LOG(LEVEL_ERR, "%s, seek off 0x%lx != 0x%lx",
                                           fname, rc, offset);

                                return -EIO;
                        }
                }

                /* read data from file into memory buffer */
                rc = read(fd, buf, bytes);
                if (rc != bytes)
                {
                        FSERVO_LOG(LEVEL_ERR, "%s, R off 0x%lx, 0x%lx != 0x%lx.\n",
                                   fname, count, rc, bytes);
                        return -EIO;
                }

                count += bytes;
                buf += bytes;
                offset += bytes;
        }

        if (count != size)
        {
                FSERVO_LOG(LEVEL_ERR, "%s, R failed 0x%lx != 0x%lx.\n",
                           fname, count, size);
                return -EIO;
        }
        return count;
}

ssize_t utils_dev_write(const char *fname, int fd, char *buffer, uint64_t size,
                        uint64_t base)
{
        ssize_t rc;
        uint64_t count = 0;
        char *buf = buffer;
        off_t offset = base;

        while (count < size)
        {
                uint64_t bytes = size - count;

                if (bytes > RW_MAX_SIZE)
                        bytes = RW_MAX_SIZE;

                if (offset)
                {
                        rc = lseek(fd, offset, SEEK_SET);
                        if (rc != offset)
                        {
                                FSERVO_LOG(LEVEL_ERR, "%s, seek off 0x%lx != 0x%lx.\n",
                                           fname, rc, offset);
                                return -EIO;
                        }
                }

                /* write data to file from memory buffer */
                rc = write(fd, buf, bytes);
                if (rc != bytes)
                {
                        FSERVO_LOG(LEVEL_ERR, "%s, W off 0x%lx, 0x%lx != 0x%lx.\n",
                                   fname, offset, rc, bytes);
                        return -EIO;
                }

                count += bytes;
                buf += bytes;
                offset += bytes;
        }

        if (count != size)
        {
                FSERVO_LOG(LEVEL_ERR, "%s, W failed 0x%lx != 0x%lx.\n",
                           fname, count, size);
                return -EIO;
        }

        return count;
}

namespace snowlake
{
#define INVALID_DEV_ID -1

#define FPGA_DOWN 0

#define FPGA_START 1

#define FPGA_CALC_START()                     \
        do                                    \
        {                                     \
                shm_write_reg(0, FPGA_DOWN);  \
                usleep(1);                    \
                shm_write_reg(0, FPGA_START); \
        } while (false)

#define FPGA_CALC_START_INDEX(reg_index)              \
        do                                            \
        {                                             \
                shm_write_reg(reg_index, FPGA_DOWN);  \
                usleep(1);                            \
                shm_write_reg(reg_index, FPGA_START); \
        } while (false)

#define FPGA_CALC_START_INDEX2(reg_index)             \
        do                                            \
        {                                             \
                shm_write_reg(1, 0x3c000000);         \
                usleep(1);                            \
                shm_write_reg(reg_index, FPGA_DOWN);  \
                usleep(1);                            \
                shm_write_reg(reg_index, FPGA_START); \
        } while (false)

        static int32 *dev_mem_addr_reg = nullptr;

        static void *dev_mem_addr_shm = nullptr;

        static void *dev_mem_addr_shm2 = nullptr;

        static int32 fd_reg_dev = INVALID_DEV_ID;

        static int32 fd_shm_dev = INVALID_DEV_ID;

        static dev_init_param_st init_param = {0};

        static boolean is_shm_dev_opened = false;

        int32 shm_open(const dev_init_param_st &param)
        {
                int32 ret = ERR_OPEN_DEV_FAILED;

                do
                {
                        FSERVO_CHECK_BREAK(false == is_shm_dev_opened, ret);

                        fd_reg_dev = open(param.reg_dev_name, O_RDWR);
                        FSERVO_CHECK_BREAK(fd_reg_dev >= 0, ret);

                        dev_mem_addr_reg = (int32 *)mmap(NULL, FPGA_REG_MAPPED_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
                                                         fd_reg_dev, 0);
                        FSERVO_CHECK_BREAK(nullptr != dev_mem_addr_reg, ret);

                        fd_shm_dev = open(param.mem_dev_name, O_RDWR);
                        FSERVO_CHECK_BREAK(fd_shm_dev >= 0, ret);

                        dev_mem_addr_shm = mmap(NULL, FPGA_MEM_MAPPED_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm_dev,
                                                FPGA_SHM_MEM_BLOCK_BASE);
                        FSERVO_CHECK_BREAK(nullptr != dev_mem_addr_shm, ret);

                        dev_mem_addr_shm2 = mmap(NULL, FPGA_MEM_MAPPED_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm_dev,
                                                 FPGA_SHM_MEM_BLOCK_BASE2);
                        FSERVO_CHECK_BREAK(nullptr != dev_mem_addr_shm2, ret);

                        FSERVO_LOG(LEVEL_DEBUG, "reg dev:%s, fid:%d, reg maped addr:%lx", param.reg_dev_name, fd_reg_dev,
                                   (uint64_t)dev_mem_addr_reg);
                        FSERVO_LOG(LEVEL_DEBUG, "shm mem dev:%s, fid:%d, shm mem maped addr:%lx", param.mem_dev_name, fd_shm_dev,
                                   (uint64_t)dev_mem_addr_shm);

                        is_shm_dev_opened = true;

                        ret = ERR_NONE;

                } while (false);

                return ret;
        }

        int32 shm_close(void)
        {
                int32 ret = ERR_NONE;

                FSERVO_CHECK(true == is_shm_dev_opened);

                //Close register opened for FPGA
                munmap((void *)dev_mem_addr_reg, FPGA_REG_MAPPED_SIZE);
                if (INVALID_DEV_ID != fd_reg_dev)
                {
                        close(fd_reg_dev);
                        fd_reg_dev = INVALID_DEV_ID;
                }
                dev_mem_addr_reg = nullptr;

                //Close shm opened for FPGA
                munmap((void *)dev_mem_addr_shm, FPGA_MEM_MAPPED_SIZE);
                dev_mem_addr_shm = nullptr;

                munmap((void *)dev_mem_addr_shm2, FPGA_MEM_MAPPED_SIZE);
                if (INVALID_DEV_ID != fd_shm_dev)
                {
                        close(fd_shm_dev);
                        fd_shm_dev = INVALID_DEV_ID;
                }
                dev_mem_addr_shm2 = nullptr;

                is_shm_dev_opened = false;

                return ret;
        }

        int32 shm_write_reg(uint32 reg_index, int32 data)
        {
                int32 ret = ERR_NONE;

                FSERVO_CHECK(true == is_shm_dev_opened);

                if (nullptr != dev_mem_addr_reg)
                {
                        dev_mem_addr_reg[reg_index] = data;
                        //            FSERVO_LOG(LEVEL_DEBUG, "write reg[%d]:0x%x, reg mapped addr:%lx", reg_index, data, (uint64)dev_mem_addr_reg);

                        ret = ERR_NORM;
                }

                return ret;
        }

        static int getRegs(int32 *mapped, uint32 reg_index, int32 &data_out)
        {
                int32 ret = ERR_REG_INDEX_INVALID;
                char *ptr = (char *)mapped;

                static const int32 int_size = sizeof(int32) * 8;
                static int index_bits = ptr[0];
                static int index_num = ptr[1];
                static int index_bytes = ptr[2];
                static unsigned int index_mask = (1 << index_bits) - 1;

                int i, index, shift_bits, mapped_index;

                for (i = 0; i < index_num; i++)
                {
                        mapped_index = ((i * index_bits) / int_size) + 1;
                        shift_bits = (i * index_bits) % int_size;

                        if (shift_bits + index_bits > int_size)
                        {
                                index = ((mapped[mapped_index] >> shift_bits) & ((1 << (int_size - shift_bits)) - 1)) | ((mapped[mapped_index + 1] & ((1 << (shift_bits + index_bits - int_size)) - 1)) << (int_size - shift_bits));
                        }
                        else
                        {
                                index = (mapped[mapped_index] >> shift_bits) & index_mask;
                        }
                        if (reg_index == index)
                        {
                                data_out = mapped[1 + index_bytes + i];
                                ret = ERR_NONE;
                                break;
                        }
                }

                return ret;
        }

        int32 shm_read_reg(uint32 reg_index, int32 &data_out)
        {
                int32 ret = ERR_NORM;

                do
                {
                        FSERVO_CHECK_BREAK(true == is_shm_dev_opened, ret);

                        FSERVO_CHECK_BREAK(nullptr != dev_mem_addr_reg, ret);

                        FSERVO_CHECK_BREAK(reg_index >= 0, ret);

                        ret = getRegs(dev_mem_addr_reg, reg_index, data_out);

                } while (false);

                return ret;
        }

        int32 shm_write(void *dst_ptr, void *data_in_ptr, int32 data_size, data_type_en type, bool batch2)
        {
                int32 ret = ERR_NORM;

                FSERVO_CHECK(true == is_shm_dev_opened);

                if (DATA_TYPE_DATA == type || DATA_TYPE_PARAM == type)
                {
                        //            FSERVO_LOG(LEVEL_DEBUG, "shm_write, dev_mem_addr_shm:%lx, dst_ptr:%lx begin", (uint64)dev_mem_addr_shm, (uint64) dst_ptr);
                        //TODO to be removed
                        if (batch2)
                        {
                                memcpy((void *)((char *)dev_mem_addr_shm2 + (uint64_t)dst_ptr), data_in_ptr, data_size);
                        }

                        else
                                memcpy((void *)((char *)dev_mem_addr_shm + (uint64_t)dst_ptr), data_in_ptr, data_size);
                        //            FSERVO_LOG(LEVEL_DEBUG, "shm_write, dev_mem_addr_shm:%lx, dst_ptr:%lx end", (uint64)dev_mem_addr_shm, (uint64) dst_ptr);
                }
                else
                {
                        ret = ERR_NONE;
                }

                if (DATA_TYPE_DATA == type)
                {
                        FPGA_CALC_START();
                }

                return ret;
        }

        int32 shm_write_size(void *dst_ptr, void *data_in_ptr, uint32 data_size, data_type_en type, int picture_resolution, bool batch2)
        {
                int32 ret = ERR_NORM;

                FSERVO_CHECK(true == is_shm_dev_opened);

                if (DATA_TYPE_DATA == type || DATA_TYPE_PARAM == type)
                {
                        if (batch2)
                                memcpy((void *)((uint64_t)dev_mem_addr_shm2 + (uint64_t)dst_ptr), data_in_ptr, (uint64_t)data_size);
                        else
                        {
#ifndef _NEON
                                memcpy((void *)((uint64_t)dev_mem_addr_shm + (uint64_t)dst_ptr), data_in_ptr, (uint64_t)data_size);
#else
                                neon_memcpy((void *)((uint64_t)dev_mem_addr_shm + (uint64_t)dst_ptr), data_in_ptr, (uint64_t)data_size);
#endif
                        }
                }
                else
                {
                        ret = ERR_NONE;
                }

                if (DATA_TYPE_DATA == type)
                {
                        FPGA_CALC_START_INDEX2(picture_resolution);
                }
                return ret;
        }

        int32 shm_read(void *src_ptr, void *data_out_ptr, int32 data_size, data_type_en type, bool batch2)
        {
                int32 ret = ERR_NORM;

                FSERVO_CHECK(true == is_shm_dev_opened);

                if (INVALID_DEV_ID != fd_reg_dev)
                {
                        if (batch2)
                                memcpy(data_out_ptr, (void *)((uint64_t)src_ptr + (uint64_t)dev_mem_addr_shm2), data_size);
                        else
                                memcpy(data_out_ptr, (void *)((uint64_t)src_ptr + (uint64_t)dev_mem_addr_shm), data_size);
                        ret = ERR_NONE;
                }

                return ret;
        }

        int32 shm_get_input_addr(void **addr)
        {
                FSERVO_CHECK(nullptr != addr);

                return ERR_NONE;
        }

        int32 shm_read_from_dev(void *dst_ptr, int dev_id, int32 data_size, data_type_en type)
        {
                int32 ret = ERR_NORM;

                FSERVO_CHECK(true == is_shm_dev_opened);

                if (DATA_TYPE_DATA == type || DATA_TYPE_PARAM == type)
                {
                        //            FSERVO_LOG(LEVEL_DEBUG, "shm_read_from_dev_id, dev_mem_addr_shm:%lx, dst_ptr:%lx, data_size:%d begin",
                        //                       dev_mem_addr_shm, dst_ptr, data_size);
                        ssize_t size = utils_dev_read("", dev_id, (char *)((uint64_t)dev_mem_addr_shm + (uint64_t)dst_ptr),
                                                      data_size, 0);
                        //            FSERVO_LOG(LEVEL_DEBUG, "shm_read_from_dev_id end");
                        FSERVO_CHECK(data_size == size);
                }
                else
                {
                        ret = ERR_NONE;
                }

                if (DATA_TYPE_DATA == type)
                {
                        FPGA_CALC_START();
                }

                return ret;
        }

        int32 shm_get_mapped_mem_addr(void **mem_addr_pptr, bool batch2)
        {
                FSERVO_CHECK(true == is_shm_dev_opened);

                FSERVO_CHECK(nullptr != mem_addr_pptr);

                if (batch2)
                        *mem_addr_pptr = dev_mem_addr_shm2;
                else
                        *mem_addr_pptr = dev_mem_addr_shm;

                return ERR_NONE;
        }

} // end of namespace snowlake
