#ifndef __BASE_TYPEDEF_H
#define __BASE_TYPEDEF_H

#ifdef __GNUC__
#include <stdint.h>
#include <inttypes.h>
#endif

#if defined(WIN32) || defined(_WIN32) ||  defined(_WIN64) ||  defined(WIN64)
#pragma warning(disable:4996)
#pragma warning(disable:4005)
#pragma warning(disable:4819)
#pragma warning(disable:4006)

#ifndef _STDINT
typedef signed char				int8_t;
#endif
#ifndef _STDINT
typedef short				int16_t;
#endif
#ifndef _STDINT
typedef int					int32_t;
#endif
#ifndef _STDINT
typedef long long			int64_t;
#endif

#ifndef _STDINT
typedef unsigned char		uint8_t;
#endif
#ifndef _STDINT
typedef unsigned short		uint16_t;
#endif
#ifndef _STDINT
typedef unsigned int		uint32_t;
#endif
#ifndef _STDINT
typedef unsigned long long	uint64_t;
#endif

#elif defined(_WIN64)
#ifndef _STDINT
typedef char				int8_t;
#endif
#ifndef _STDINT
typedef short				int16_t;
#endif
#ifndef _STDINT
typedef int					int32_t;
#endif
#ifndef _STDINT
typedef long				int64_t;
#endif

#ifndef _STDINT
typedef unsigned char		uint8_t;
#endif
#ifndef _STDINT
typedef unsigned short		uint16_t;
#endif
#ifndef _STDINT
typedef unsigned int		uint32_t;
#endif
#ifndef _STDINT
typedef unsigned long		uint64_t;
#endif
#endif

#ifndef core_max
#define core_max(a, b)		((a) > (b) ? (a) : (b))
#endif

#ifndef core_min
#define core_min(a, b)		((a) < (b) ? (a) : (b))
#endif

#ifndef core_abs
#define core_abs(a, b)		((a) > (b) ? (a - b) : (b - a))
#endif

#ifndef NULL
#define NULL	0
#endif


#ifdef WIN32

#ifndef CONNECTION_POOL_SIZE
#define CONNECTION_POOL_SIZE		32
#endif

#ifndef LOG_POOL_SIZE
#define LOG_POOL_SIZE				32
#endif

#ifndef TIMER_POOL_SIZE
#define TIMER_POOL_SIZE				32
#endif

#ifndef STREAM_POOL_SIZE
#define STREAM_POOL_SIZE			16
#endif



#else

#ifndef CONNECTION_POOL_SIZE
#define CONNECTION_POOL_SIZE		16
#endif

#ifndef LOG_POOL_SIZE
#define LOG_POOL_SIZE				32
#endif

#ifndef TIMER_POOL_SIZE
#define TIMER_POOL_SIZE				4
#endif

#ifndef STREAM_POOL_SIZE
#define STREAM_POOL_SIZE			96
#endif

#endif

//定义回话ID
typedef 			uint32_t        SessionID;

#endif
