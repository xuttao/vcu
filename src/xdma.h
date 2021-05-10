#ifndef _FSERVO_XDMA_H_
#define _FSERVO_XDMA_H_

#include "devManager.h"
#include "typesdef.h"
#include <stdint.h>
#include <sys/types.h>

namespace snowlake
{

        int32 xdma_open(const dev_init_param_st &param);

        int32 xdma_close(void);

        int32 xdma_write(void *dst_ptr, void *data_in_ptr, int32 data_size, data_type_en type, bool batch2);

        int32 xdma_write_size(void *dst_ptr, void *data_in_ptr, uint32 data_size, data_type_en type, int picture_resolution, bool batch2);

        int32 xdma_read(void *src_ptr, void *data_out_ptr, int32 data_size, data_type_en type, bool batch2);

        int32 xdma_write_reg(uint32 reg_index, int32 data);

        int32 xdma_read_reg(uint32 reg_index, int32 &data_out);

        int32 xdma_read_from_dev(void *dst_ptr, int dev_id, int32 data_size, data_type_en type);

        int32 xdma_get_mapped_mem_addr(void **mem_addr_pptr, bool batch2);

        //define for read and write XDMA device(refer to dma_utils.c)
        extern "C"
        {
                ssize_t read_to_buffer(char *fname, int fd, char *buffer, uint64_t size, uint64_t base);
                ssize_t write_from_buffer(char *fname, int fd, char *buffer, uint64_t size, uint64_t base);
        }

} // end of namespace snowlake

#endif //_FSERVO_XDMA_H_
