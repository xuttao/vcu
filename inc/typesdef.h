//
// Created by leo on 5/16/19.
//

#ifndef _FSERVO_TYPESDEF_H_
#define _FSERVO_TYPESDEF_H_

#ifdef __cplusplus
namespace snowlake {
#endif

/* -----------------------------------------------------------------------
** Standard Types
** ----------------------------------------------------------------------- */

/* The following definitions are the same accross platforms.  This first
** group are the sanctioned types.
*/

/* Boolean value type. */
#ifndef _BOOLEAN_DEFINED
    typedef unsigned char boolean;
#define _BOOLEAN_DEFINED
#endif

/* cchr value type. */
#ifndef _CCHR_DEFINED
    typedef const char cchr;
#define _BOOL_DEFINED
#endif

/* Unsigned 64 bit value */
// #ifndef _UINT64_DEFINED
//     typedef unsigned long uint64;
// #define _UINT64_DEFINED
// #endif

/* Unsigned 32 bit value */
#ifndef _UINT32_DEFINED
    typedef unsigned int uint32;
#define _UINT32_DEFINED
#endif

/* Unsigned 16 bit value */
#ifndef _UINT16_DEFINED
    typedef unsigned short uint16;
#define _UINT16_DEFINED
#endif

/* Unsigned 8  bit value */
#ifndef _UINT8_DEFINED
    typedef unsigned char uint8;
#define _UINT8_DEFINED
#endif


// #ifndef _INT64_DEFINED
//     typedef signed long int64;
// #define _INT64_DEFINED
// #endif

/* Signed 32 bit value */
#ifndef _INT32_DEFINED
    typedef signed int int32;
#define _INT32_DEFINED
#endif

/* Signed 16 bit value */
#ifndef _INT16_DEFINED
    typedef signed short int16;
#define _INT16_DEFINED
#endif

/* Signed 8  bit value */
#ifndef _INT8_DEFINED
    typedef signed char int8;
#define _INT8_DEFINED
#endif

#ifndef _cstring_DEFINED
    typedef char *cstring;
#define _cstring_DEFINED
#endif

#ifndef _ucahr_DEFINED
    typedef uint16 unicode;
#define _ucahr_DEFINED
#endif

#ifndef _ustring_DEFINED
    typedef unicode *ustring;
#define _ustring_DEFINED
#endif

#ifndef _AECHAE_DEFINED
    typedef unicode WCHAR;
#define _AECHAE_DEFINED
#endif

#ifndef  FALSE
#define  FALSE              0
#endif

#ifndef  TRUE
#define  TRUE               1
#endif

//    typedef enum _error_code_ : uint8 {
    typedef enum _error_code_ {

        //ok
        ERR_NONE = 0,

        //normal error
        ERR_NORM,

        //invalid parameter error
        ERR_PARAM,

        //device error
        ERR_BUSY,

        //failed to config FPGA
        ERR_PARA,

        //AIX bus error
        ERR_AXI,

        ERR_EMPTY,

        ERR_OPEN_DEV_FAILED,

        ERR_REG_INDEX_INVALID,

        ERR_FPGA_CAL_FINISHED,

    } error_code_en;


//    typedef enum class _cmd_type_ : uint8 {
    typedef enum _cmd_type_ {

        CMD_INIT_FPGA = 1,

        CMD_START_FPGA_CALCULATE,

        CMD_STOP_FPGA,

    } cmd_type_en;

#ifdef __cplusplus
} // end of namespace snowlake
#endif

#endif //_FSERVO_TYPESDEF_H_
