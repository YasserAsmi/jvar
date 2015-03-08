/**
 * @file include/util.h
 * Declares various utility preprocessor symbols, types, and classes.
 * @copyright Copyright (c) 2014 Yasser Asmi; Released under the MIT
 *            License (http://opensource.org/licenses/MIT)
 */

#ifndef _UTIL_H
#define _UTIL_H

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS

#pragma warning(disable: 4521)

#endif

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <memory.h>
#include <errno.h>

#include <string>

#ifndef NDEBUG
    /**
     * We are debugging
     */
#ifndef _DEBUG    
    #define _DEBUG
#endif
    /**
     * Log an info message
     */
    #define dbglog(fmt, ...) \
        do { fprintf(stderr, fmt, ## __VA_ARGS__); } while (0)

    /**
     * Log an error message
     */
    #define dbgerr(fmt, ...) \
        do { fprintf(stderr, "Error: %s(%d): " fmt, __FILE__, __LINE__, ## __VA_ARGS__); } while (0)

    /**
     * Debug trace if enabled
     */
    #define dbgtrc(fmt, ...) \
        do { if (jvar::enable_dbgtrc) fprintf(stderr, "%s(%d): " fmt, __func__, __LINE__, ## __VA_ARGS__); } while (0)

    /**
     * Write a message with the function and file
     */
    #define dbgfnc() \
        do { fprintf(stderr, "%s in %s(%d)\n", __PRETTY_FUNCTION__, __FILE__, __LINE__); } while (0)

namespace jvar
{
    /**
     * Hex dumper
     */
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

/**
 * Is the given \p flag set in \p value?
 */
#define isFlagSet(value, flag)         ( ((value) & (flag)) != 0 )

/**
 * Is the given \p flag not set in \p value?
 */
#define isFlagClear(value, flag)       ( ((value) & (flag)) == 0 )

/**
 * Set the \p flag into \p value.
 */
#define setFlag(value, flag)           { (value) |= (flag); }

/**
 * Clear the \p flag out of \p value.
 */
#define clearFlag(value, flag)         { (value) &= ~(flag); }

/**
 * Array length count
 */
#define countof(x)  ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

/*
 * Windows specific
*/

#ifdef _MSC_VER

    #define snprintf _snprintf 
    #define vsnprintf _vsnprintf 
    #define strcasecmp _stricmp 
    #define strncasecmp _strnicmp 

#ifndef va_copy
    #define va_copy(dest, src) ((dest) = (src))
#endif

inline bool isnan(double x) 
{ 
    return x != x; 
}
#endif


namespace jvar
{

typedef unsigned char uchar; ///< Shorthand type definition
typedef short int shortint; ///< Shorthand type definition
typedef unsigned short int ushortint; ///< Shorthand type definition
typedef unsigned int uint; ///< Shorthand type definition
typedef long int longint; ///< Shorthand type definition
typedef unsigned long int ulongint; ///< Shorthand type definition

/**
 * Enable debug trace (unused?)
 */
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

/**
 * The interface to an implementation.
 */
class InterfaceImpl
{
public:
    virtual ~InterfaceImpl()
    {
    }

    /**
     * Pure virtual factory method.
     */
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

