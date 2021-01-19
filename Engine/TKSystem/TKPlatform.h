#ifndef TK_PLATFORM_H
#define TK_PLATFORM_H

// namespace macro
#define TK_NAMESPACE_BEGIN		namespace TKEngine {
#define TK_NAMESPACE_END        }
#define USING_TK_NAMESPACE      using namespace TKEngine;

#ifdef TKSYSTEM_EXPORTS
#define	TKSYSTEM_API __declspec(dllexport)
#else
#define TKSYSTEM_API __declspec(dllimport)
#endif

// operating system type
#define TK_PLATFORM_WIN 1
#define TK_PLATFORM_LINUX 2
#define TK_PLATFORM_MAC 3
#define TK_PLATFORM_ANDROID 4
#define TK_PLATFORM_APPLE_IOS 5

// complier type
#define TK_COMPILER_MSVC 1
#define TK_COMPLIER_GUNC 2
#define TK_COMPILER_GCCE 3

#define TK_ENDIAN_LITTLE 1
#define TK_ENDIAN_BIG 2

// Finds the current platform.
#if defined(__WIN32__) || defined(_WIN32) || defined(_WINDOWS) || defined(WIN) || defined(_WIN64) || defined(__WIN64__)
#   define TK_PLATFORM TK_PLATFORM_WIN
#elif defined( __APPLE_CC__) || defined(__APPLE__) || defined(__OSX__)
// Device
// Both requiring OS version 4.0 or greater
#   if __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ >= 40000 || __IPHONE_OS_VERSION_MIN_REQUIRED >= 40000
#       define TK_PLATFORM TK_PLATFORM_APPLE_IOS
#   else
#       define TK_PLATFORM TK_PLATFORM_MAC
#   endif
#elif defined(__ANDROID__)
#   define TK_PLATFORM TK_PLATFORM_ANDROID
#else
#   define TK_PLATFORM TK_PLATFORM_LINUX
#endif

// Finds the compiler type and version.
#if defined(__GCCE__)
#   define TK_COMPILER TK_COMPILER_GCCE
#   define TK_COMP_VER _MSC_VER
#elif defined(_MSC_VER)
#   define TK_COMPILER TK_COMPILER_MSVC
#   define TK_COMP_VER _MSC_VER
#elif defined(__GNUC__)
#   define TK_COMPILER TK_COMPLIER_GUNC
#   define TK_COMP_VER (((__GNUC__)*100) + \
                        (__GNUC_MINOR__*10) + \
                        __GNUC_PATCHLEVEL__)
#else
#   pragma error "No known compiler. Abort! Abort!"
#endif

// See if we can use __forceinline or if we need to use __inline instead
#if TK_COMPILER == TK_COMPILER_MSVC
#   if TK_COMP_VER >= 1200
#       define FORCEINLINE __forceinline
#   endif
#elif defined(__MINGW32__)
#   if !defined(FORCEINLINE)
#       define FORCEINLINE __inline
#   endif
#else
#   define FORCEINLINE __inline
#endif


#if defined(__WIN32__) || defined(_WIN32)
#define SYSTEM_BIT_WIDTH 32
#else
#define SYSTEM_BIT_WIDTH 64
#endif

#endif //TK_PLATFORM_H

