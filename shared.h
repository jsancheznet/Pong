/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <jsanchez@monoinfinito.net> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Jorge Sánchez
 * ----------------------------------------------------------------------------
 */

#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

#pragma warning(disable: 4189)
#pragma warning(disable: 4201)
#pragma warning(disable: 4100)

typedef Uint8     u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef float    r32;
typedef double   r64;
typedef i32      b32;

#define global_variable static
#define internal_variable static

#define INLINE __forceinline
#define Assert(Expr) assert(Expr)

#ifdef _WIN32
#define Free(Expr) VirtualFree((Expr), 0, MEM_RELEASE)
#define Malloc(Size) VirtualAlloc(NULL, (SIZE_T)(Size), MEM_COMMIT, PAGE_READWRITE)
#define ZeroMem(Ptr, Size) ZeroMemory(Ptr, Size)
#define ZeroArray(Ptr, Size) ZeroMemory(Ptr, Size)
#endif

#define Kilobytes(Expr) ((Expr) * 1024)
#define Megabytes(Expr) (Kilobytes(Expr) * 1024)
#define Gigabytes(Expr) (Megabytes(Expr) * 1024)

//
// Error Checking Macro's
//

#define NULL_CHECK(Expr)                                                                                    \
    if(!Expr)                                                                                               \
    {                                                                                                       \
        DWORD Err = GetLastError();                                                                         \
        printf("NULL_CHECK TRIGGERED, MSDN ErrorCode: %ld, File: %s Line: %d \n", Err, __FILE__, __LINE__); \
        return NULL;                                                                                        \
    }

#define GLERR                                                                                               \
    {                                                                                                       \
        GLuint glerr;                                                                                       \
        while((glerr = glGetError()) != GL_NO_ERROR)                                                        \
            fprintf(stderr, "%s:%d glGetError() = 0x%04x\n", __FILE__, __LINE__, glerr);                      \
    }

void Clamp(r32 *Value, r32 Lower, r32 Upper)
{
    if(*Value > Upper)
    {
        *Value = Upper;
    }
    else if(*Value < Lower)
    {
        *Value = Lower;
    }
}
