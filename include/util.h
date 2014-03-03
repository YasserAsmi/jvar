// Copyright (c) 2014 Yasser Asmi
// Released under the MIT License (http://opensource.org/licenses/MIT)

#ifndef _UTIL_H
#define _UTIL_H

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <memory.h>
#include <errno.h>

#include <string>

#ifndef NDEBUG
    #define _DEBUG

    #define dbglog(fmt, ...) \
        do { fprintf(stderr, fmt, ## __VA_ARGS__); } while (0)

    #define dbgerr(fmt, ...) \
        do { fprintf(stderr, "Error: %s(%d): " fmt, __FILE__, __LINE__, ## __VA_ARGS__); } while (0)

    #define dbgtrc(fmt, ...) \
        do { if (jvar::enable_dbgtrc) fprintf(stderr, "%s(%d): " fmt, __func__, __LINE__, ## __VA_ARGS__); } while (0)

    #define dbgfnc() \
        do { fprintf(stderr, "%s in %s(%d)\n", __PRETTY_FUNCTION__, __FILE__, __LINE__); } while (0)

namespace jvar
{
    void dbghex(const char* label, const void* ptr, int len);
}

#else
    #undef _DEBUG

    #define dbglog(fmt, ...)

    #define dbgerr(fmt, ...) \
        do { fprintf(stderr, "Error: " fmt, ## __VA_ARGS__); } while (0)

    #define dbgtrc(fmt, ...)

    #define dbgfnc()

    #define dbghex(label, ptr, len)
#endif

#define isFlagSet(value, flag)         ( ((value) & (flag)) != 0 )
#define isFlagClear(value, flag)       ( ((value) & (flag)) == 0 )
#define setFlag(value, flag)           { (value) |= (flag); }
#define clearFlag(value, flag)         { (value) &= ~(flag); }

#define countof(x)  ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))


namespace jvar
{

typedef unsigned char uchar;
typedef short int shortint;
typedef unsigned short int ushortint;
typedef unsigned int uint;
typedef long int longint;
typedef unsigned long int ulongint;

extern bool enable_dbgtrc;

/**
 * Buffer class maintains an allocated chunk of memory.  It takes care of freeing the memory
 * when the object goes out of scope.  It uses malloc/free/realloc.   It can also read a
 * file into the buffer.
 */
class Buffer
{
public:
    Buffer() :
        mMemory(NULL),
        mSize(0)
    {
    }

    ~Buffer()
    {
        free();
    }

    /**
     * Returns a pointer to the memory
     */
    inline void* ptr()
    {
        return mMemory;
    }

    /**
     * Returns a const pointer to the memory
     */
    inline const void* cptr()
    {
        return mMemory;
    }

    /**
     * Returns the size of the buffer
     *
     * @return Number of bytes
     */
    inline size_t size() const
    {
        return mSize;
    }

    /**
     * Allocates memory.  If previously allocated, the memory is freed first.
     *
     * @param size Size in bytes
     */
    void alloc(size_t size);

    /**
     * Realocates the buffer to be the new size.  realloc() semantics are used.
     *
     * @param size New size in bytes
     */
    void reAlloc(size_t size);

    /**
     * Frees the memory
     */
    void free();

    /**
     * Copies the memory from the provided buffer into this buffer
     *
     * @param src Buffer to copy memory from
     */
    void copyFrom(Buffer& src);

    /**
     * Moves the memory from the provided buffer into this buffer.  The provided
     * buffer is emptied.  No actual memcpy is done.
     *
     * @param src Buffer to move memory from
     */
    void moveFrom(Buffer& src);

    /**
     * Reads a file into the buffer
     *
     * @param  filename Name of the file to read
     * @param  nullterm If True, add a '\0' so the buffer can be used as a null-terminated string
     *
     * @return          Success
     */
    bool readFile(const char* filename, bool nullterm);

private:
    void* mMemory;
    size_t mSize;
};


class InterfaceImpl
{
public:
    virtual ~InterfaceImpl()
    {
    }
    virtual InterfaceImpl* newImpl() = 0;
};

/**
 * Returns the current clock in miliseconds
 *
 * @return Miliseconds
 */
ulongint getTickCount();


} // jvar

#endif // _UTIL_H

