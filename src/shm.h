#ifndef _FSERVO_SHM_H_
#define _FSERVO_SHM_H_

#include "devManager.h"
#include "typesdef.h"
#include <stdint.h>
#include <sys/types.h>

namespace snowlake
{

        int32 shm_open(const dev_init_param_st &param);

        int32 shm_close(void);

        int32 shm_write(void *dst_ptr, void *data_in_ptr, int32 data_size, data_type_en type, bool batch2 = false);

        int32 shm_write_size(void *dst_ptr, void *data_in_ptr, uint32 data_size, data_type_en type, int picture_resolution, bool batch2 = false);

        int32 shm_read(void *src_ptr, void *data_out_ptr, int32 data_size, data_type_en type, bool batch2 = false);

        int32 shm_write_reg(uint32 reg_index, int32 data);

        int32 shm_read_reg(uint32 reg_index, int32 &data_out);

        int32 shm_read_from_dev(void *dst_ptr, int dev_id, int32 data_size, data_type_en type);

        int32 shm_get_mapped_mem_addr(void **mem_addr_pptr, bool batch2 = false);

} // end of namespace snowlake

#endif //_FSERVO_SHM_H_
