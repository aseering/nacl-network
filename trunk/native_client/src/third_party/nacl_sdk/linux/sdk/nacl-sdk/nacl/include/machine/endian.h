#ifndef __MACHINE_ENDIAN_H__

#include <sys/config.h>

#ifndef BIG_ENDIAN
#define BIG_ENDIAN 4321
#define __BIG_ENDIAN 4321
#endif
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1234
#define __LITTLE_ENDIAN 1234
#endif

#ifndef BYTE_ORDER
#ifdef __IEEE_LITTLE_ENDIAN
#define BYTE_ORDER LITTLE_ENDIAN
#define __BYTE_ORDER __LITTLE_ENDIAN
#else
#define BYTE_ORDER BIG_ENDIAN
#define __BYTE_ORDER __BIG_ENDIAN
#endif
#endif

#endif /* __MACHINE_ENDIAN_H__ */
