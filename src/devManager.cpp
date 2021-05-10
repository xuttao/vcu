#include "devManager.h"
#include "log.h"
#include "shm.h"
#include "xdma.h"
#include <string.h>

namespace snowlake
{

        const static dev_op_st shared_mem = {
            .name = DEV_NAME_SHARE_MEM,
            .open = shm_open,
            .close = shm_close,
            .write = shm_write,
            .write_size = shm_write_size,
            .read = shm_read,
            .write_reg = shm_write_reg,
            .read_reg = shm_read_reg,
            .read_from_dev = shm_read_from_dev,
            .get_mapped_mem_addr = shm_get_mapped_mem_addr,
        };

        const static dev_op_st xdma = {
            .name = DEV_NAME_XDMA,
            .open = xdma_open,
            .close = xdma_close,
            .write = xdma_write,
            .write_size = xdma_write_size,
            .read = xdma_read,
            .write_reg = xdma_write_reg,
            .read_reg = xdma_read_reg,
            .read_from_dev = xdma_read_from_dev,             // 未实现
            .get_mapped_mem_addr = xdma_get_mapped_mem_addr, // 未实现
        };

        //Config global device table
        const static dev_op_st devices_cfg_table[DEV_NUM_MAX] = {
            shared_mem,
            xdma,
        };

        const dev_op_st *dev_init(const char *dev_name)
        {
                const dev_op_st *dev_ptr = nullptr;
                int32 dev_count = get_supported_dev_cnt();

                for (int32 i = 0; i < dev_count; ++i)
                {
                        if ((dev_name != nullptr) &&
                            (strcmp(dev_name, get_supported_dev_name(i)) == 0))
                        {
                                dev_ptr = &(devices_cfg_table[i]);
                                break;
                        }
                }

                FSERVO_CHECK(NULL != dev_ptr);

                return dev_ptr;
        }

        int32 get_supported_dev_cnt(void)
        {
                return sizeof(devices_cfg_table) / sizeof(dev_op_st);
        }

        const char *get_supported_dev_name(int32 dev_index)
        {
                const char *dev_name = NULL;
                bool index_is_valid = (dev_index < get_supported_dev_cnt());

                FSERVO_CHECK(index_is_valid);

                if (index_is_valid)
                {
                        dev_name = devices_cfg_table[dev_index].name;
                }
                return dev_name;
        }

} // end of namespace snowlake
