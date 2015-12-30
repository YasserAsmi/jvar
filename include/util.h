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
#pragma warning(disable: 4018)

#endif

#ifdef __CYGWIN__ 
// This define blocks some functions such as strptime() that are required
#undef __STRICT_ANSI__
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

#ifndef _MSC_VER
#include <unistd.h>
#endif
//#include <stdint.h>

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
        do { fprintf(stderr, "fn: %s\n", __PRETTY_FUNCTION__); } while (0)

    /**
     * Debug trace a message if condition x is true (similar to assert--but does not break)
     */
    #define dbgtru(x) \
        do { if (!(x)) {printf("Error: Not true: '" #x "' %s(%d)\n", __FILE__, __LINE__); } } while (0)

namespace jvar
{
    /**
     * Hex dumper
     */
    void dbghex(const char* label, const void* ptr, int len);

    /**
     * Prints system error message associated with error no.
     */
    void printeno(int eno, const char* func);
}
    /**
     * Debug trace a systemerror message
     */
    #define dbgeno(eno) \
        do { if (eno) {jvar::printeno(eno, __PRETTY_FUNCTION__);} } while (0)

#else
    #undef _DEBUG

    #define dbglog(fmt, ...)

    #define dbgerr(fmt, ...) \
        do { fprintf(stderr, "Error: " fmt, ## __VA_ARGS__); } while (0)

    #define dbgtrc(fmt, ...)

    #define dbgfnc()

    #define dbgtru(x)

    #define dbghex(label, ptr, len)

namespace jvar
{
    void printeno(int eno, const char* func);
}

    #define dbgeno(eno) \
        do { if (eno) {jvar::printeno(eno, NULL);} } while (0)

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

/**
 * Compile time assert
 */
#define cassert(e) extern char (*COMPILE_TIME_ASSERT()) [sizeof(char[1 - 2*!(e)])]

/**
 * Returns address of an object when the & operator is overloaded
 */
template <class T>
T* addressOf(T& v)
{
    return reinterpret_cast<T *>(& const_cast<char&>(reinterpret_cast<const volatile char &>(v)));
}

// Windows specific

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

#ifndef va_copy
    #define va_copy(dest, src) ((dest) = (src))
#endif



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
    Buffer(size_t size) :
        mMemory(NULL),
        mSize(0)
    {
        alloc(size);
    }
    Buffer(const Buffer& src) :
        mMemory(NULL),
        mSize(0)
    {
        copyFrom(src);
    }
    Buffer(Buffer& src) :
        mMemory(NULL),
        mSize(0)
    {
        copyFrom(src);
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
    inline const void* cptr() const
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

    void dblOr(size_t neededsize);

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
    void copyFrom(const Buffer& src);

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

    void zero()
    {
        memset(mMemory, 0, mSize);
    }

private:
    void* mMemory;
    size_t mSize;
};


/**
 * Iter class template is used to iterate over an array as follows:
 * \code
 *    for (Iter<Obj> i; arr.forEach(i); )
 *    {
 *        foo(i->field);
 *    }
 * \endcode
 */
template <class T>
class Iter
{
public:
    Iter() :
        mPos(-1),
        mObj(NULL),
        mKey(NULL),
        mPtr(NULL)
    {
    }
    /**
     * @return   Returns a reference to the current element
     */
    inline T* operator->()
    {
        return mObj;
    }
    /**
     * @return   Dereferences the current element
     */
    inline T& operator*()
    {
        return *mObj;
    }
    /**
     * @return   Returns a pointer to the current element
     */
    inline T* operator&()
    {
        return mObj;
    }
    /**
     * @return Returns the position of of the current element in the iterator
     */
    inline int pos()
    {
        return mPos;
    }
    /**
     * @return Returns the key for the current element if available
     */
    inline const char* key()
    {
        return mKey;
    }

public:
/** \cond INTERNAL */
    int mPos;
    T* mObj;
    const char* mKey;
    void* mPtr;
/** \endcond */
};


class Date : public tm
{
public: //API

    // Exposed directly from tm:
    //  tm_sec
    //  tm_min
    //  tm_hour
    //  tm_mday
    //  tm_mon
    //  tm_year
    //  tm_wday
    //  tm_yday
    //  tm_isdst

    Date()
    {
        zero();
    }
    Date(const char* str, const char* fmt = NULL)
    {
        (void)parse(str, fmt);
    }
    explicit Date(time_t utc)
    {
        gmtime_r(&utc, this);
    }

    void now()
    {
        time_t enow;
        time(&enow);
        localtime_r(&enow, this);
    }
    void zero()
    {
        memset(this, 0, sizeof(tm));
    }
    void normalize()
    {
        (void)mktime(this);
    }

    std::string toString(const char* fmt = NULL) const
    {
        char buf[128];
        strftime(buf, sizeof(buf), fmt == NULL ? stdFmt() : fmt, this);
        return std::string((const char*)buf);
    }
    bool parse(const char* str, const char* fmt = NULL)
    {
        return (strptime(str, fmt == NULL ? stdFmt() : fmt, this) != NULL);
    }

    const char* stdFmt() const
    {
        return "%a, %d %b %Y %H:%M:%S GMT";
    }

    time_t utc() const
    {
        return timegm((tm*)this);
    }

    longint secondsSince(time_t sinceutc) const
    {
        // returns this - sinceutc
        return (longint)difftime(utc(), sinceutc);
    }
    longint secondsSince(const Date& sinceutc) const
    {
        // returns this - sinceutc
        return (longint)difftime(utc(), sinceutc.utc());
    }
};

/**
 * Returns time string for now()
 *
 * @param  fmt Format of the time
 */
std::string nowStr(const char* fmt = NULL);

/**
 * Returns the current clock in milliseconds
 *
 * @return Milliseconds
 */
ulongint getTickCount();

/**
 * Sleeps for milliseconds
 *
 * @param millisecs Duration of sleep
 */
inline void sleep(int millisecs)
{
    usleep(millisecs * 1000);
}

/**
 * Add milliseconds to timespec
 *
 * @param ts        Pointer to timespec
 * @param millisecs Milliseconds to add
 */
void tsAddMsecs(struct timespec* ts, longint millisecs);

/**
 * Automatically seeds the first time it is called and returns a random
 * between zero and max.
 *
 * NOTE: Not thread-safe
 *
 * @param  max Max random number
 *
 * @return     A random integer between 0 and Max (excluded)
 */
int random(int max);


/** \cond INTERNAL */

// Interface macros

#define INTF_NAME(x) \
    virtual const char* interfaceName() {return x; }

#define INTF_CAST(xx) \
    static uint iid() { return __COUNTER__; } \
    virtual bool isCastable(uint iid) { xx return false;}

#define IID(xx) \
    if (iid == xx::iid()) {return true;}

//  .BaseInterface is the class all interfaces should derive from
//  .Each interface, under the cover, automatically gets an unique id using __COUNTER__
//  .Each interface should have a name defined with INTF_NAME() macro
//  .Each interface should define what it cab be casted to using INTF_CAST() and IID() macros
//  .Interface pointers can be passed around freely (there is no ref counting)
//  .cast<> should be used to cast an interface from base to the desired interface
//  .cast<> can return NULL if interface is not castable

class BaseInterface
{
public:
    INTF_NAME("BaseInterface");
    INTF_CAST(
        IID(BaseInterface)
        );

    virtual ~BaseInterface()
    {
    }
    virtual void release()
    {
    }
    inline bool is(uint iid)
    {
        return isCastable(iid);
    }
    inline bool is(const char* iname)
    {
        return strcmp(iname, interfaceName()) == 0;
    }
};


template<typename T>
T* cast(BaseInterface* bp)
{
    if (bp)
    {
        if (bp->isCastable(T::iid()))
        {
            return (T*)(bp);
        }
        dbgerr("Cast failed from %s\n", bp->interfaceName());
    }
    return NULL;
}


// RcLife - Ref counted lifetime or non-intrusive ref counted object

template <class T>
class RcLife
{
public:
    RcLife() :
        mOC(NULL)
    {
    }
    explicit RcLife(T* p) :
        mOC(NULL)
    {
        if (p)
        {
            // T* provided is a 'newed' object.. and will be maintained by this function.
            // It will allocate a new obj/cnt structure and copy the object pointer into it
            // Ref count will be 1.  Object will be deleted when refcount reaches zero unless
            // noDelete(true) is called.

            mOC = new ObjAndCnt(p);
        }
    }
    RcLife(const RcLife& src)
    {
        addRef(src.mOC);
    }
    ~RcLife()
    {
        delRef();
    }

    void setNew(T* p)
    {
        // Same as the constructor with T*.  Allocate a new obj/cnt structure and
        // copy the object pointer into it.  Ref count will be 1

        delRef();

        mOC = new ObjAndCnt(p);
    }
    void release()
    {
        delRef();
    }

    void assign(const RcLife& src)
    {
        if (this != &src)
        {
            delRef();
            addRef(src.mOC);
        }
    }
    RcLife& operator= (const RcLife& src)
    {
        assign(src);
        return *this;
    }

    bool isNull()
    {
        return mOC != NULL;
    }

    T* ptr()
    {
        return mOC ? mOC->mObjPtr : NULL;
    }

    //TODO: remove this
    void noDelete(bool b)
    {
        // On rare occasions, if we should not call delete on the ref counted object
        // use this function to disable delete (b = false).  NOTE: Unless you are calling
        // delete yourself elsewhere, there could be memory leaks.

        if (mOC)
        {
            mOC->mNoDelPtr = b;
        }
    }

private:
    class ObjAndCnt
    {
    public:
        ObjAndCnt(T* p) :
            mObjPtr(p),
            mRefCnt(1),
            mNoDelPtr(false)
        {
        }
    public:
        T* mObjPtr;
        volatile int mRefCnt;
        bool mNoDelPtr;

    private:
        ObjAndCnt();
    };
    ObjAndCnt* mOC;

    void addRef(ObjAndCnt* oc)
    {
        mOC = oc;
        if (mOC)
        {
            mOC->mRefCnt++;
        }
    }
    void delRef()
    {
        if (mOC)
        {
            mOC->mRefCnt--;
            if (mOC->mRefCnt <= 0)
            {
                // Free the object (unless we were asked not to free)
                if (mOC->mNoDelPtr == false)
                {
                    delete mOC->mObjPtr;
                }

                // Free the obj/cnt.
                delete mOC;
                mOC = NULL;
            }
        }
    }
};
/** \endcond */

} // jvar

#endif // _UTIL_H

