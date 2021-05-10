#ifndef _FSERVO_DEV_MANAGER_H_
#define _FSERVO_DEV_MANAGER_H_

#include "typesdef.h"

namespace snowlake
{

#define DEV_NAME_SHARE_MEM "SHM"

        // #define  DEV_NAME_SIMULATOR   "SIMULATOR"

#define DEV_NAME_XDMA "XDMA"

#define DEV_NUM_MAX 20

        typedef enum _data_type_
        {

                DATA_TYPE_DATA = 0,

                DATA_TYPE_PARAM = 1,

        } data_type_en;

        typedef struct _dev_init_param_
        {

                const char *mem_dev_name;

                const char *reg_dev_name;

        } dev_init_param_st;

        typedef struct _dev_op_
        {

                const char *name;

                int32 (*open)(const dev_init_param_st &param);

                int32 (*close)(void);

                int32 (*write)(void *dst_ptr, void *data_in_ptr, int32 data_size, data_type_en type, bool batch2);

                int32 (*write_size)(void *dst_ptr, void *data_in_ptr, uint32 data_size, data_type_en type, int picture_resolution, bool batch2);

                int32 (*read)(void *src_ptr, void *data_out_ptr, int32 data_size, data_type_en type, bool batch2);

                int32 (*write_reg)(uint32 reg_index, int32 data);
                ;

                int32 (*read_reg)(uint32 reg_index, int32 &data_out);

                int32 (*read_from_dev)(void *dst_ptr, int dev_id, int32 data_size, data_type_en type);

                int32 (*get_mapped_mem_addr)(void **mem_addr_pptr, bool batch2);

        } dev_op_st;

        const dev_op_st *dev_init(const char *dev_name);

        int32 get_supported_dev_cnt(void);

        const char *get_supported_dev_name(int32 dev_index);

} // end of namespace snowlake

#endif //_FSERVO_DEV_MANAGER_H_
