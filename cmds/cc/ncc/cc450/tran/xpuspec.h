
/* C compiler file xpuspec.h :  Copyright (C) A.Mycroft and A.C.Norman */
/* version 7b */

#ifndef _xpuspec_LOADED
#define _xpuspec_LOADED 1

#define TRANSPUTER 1
#define TARGET_IS_XPUTER 1
#define TARGET_MACHINE "TRANSPUTER"

#ifndef SOFTWARE_FLOATING_POINT
#   define TARGET_HAS_370_FP 1
#endif

/*
#define TARGET_PREDEFINES \
   { "__transputer", "__CLK_TCK=38400", "__JMP_BUF_SIZE=8", "__EXIT_FAILURE=1" }
*/

#define TARGET_HAS_MULTIPLY 1
#define TARGET_HAS_DIVIDE   1
#define TARGET_HAS_FP_LITERALS 1
#define TARGET_HAS_BLOCKMOVE 1
#define TARGET_HAS_SWITCH_BRANCHTABLE 1
#define TARGET_HAS_2ADDRESS_CODE 1
#define TARGET_CALL_USES_DESCRIPTOR 1
/* #define TARGET_HAS_HALFWORD_INSTRUCTIONS 1 */
/* #define TARGET_HAS_TAILCALL 1 */
#define NO_INSTORE_FILES 1

#define NO_CONFIG 1
#define mcdep_config_option(x,y) 1

#define GAP 0

#define TARGET_LDRK_MIN                 0L
#define TARGET_LDRK_MAX                 0xffffffffL

#define sizeof_int      4
#define alignof_int     4
#define alignof_double  4      /* maybe make 8 soon */
#define alignof_struct  4

#endif

/* end of xpuspec.h */
 
