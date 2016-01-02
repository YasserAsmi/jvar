// Copyright (c) 2014 Yasser Asmi
// Released under the MIT License (http://opensource.org/licenses/MIT)

#include "util.h"

#ifdef __APPLE__
#include <sys/timeb.h>
#endif

namespace jvar
{

void printeno(int eno, const char* func)
{
    printf("Error: '%s(%d)' %s \n", strerror(eno), eno, func == NULL ? "" : func);
}

#ifdef _DEBUG

bool enable_dbgtrc = false;

void dbghex(const char* label, const void* ptr, int len)
{
    const unsigned char* buf = (const unsigned char*)ptr;

    if (label)
    {
        dbglog("%s hexdump %p %x(%d) byte(s):\n", label, ptr, len, len);
    }

    for (int i = 0; i < len; i += 16)
    {
        dbglog("%06x: ", i);
        for (int j = 0; j < 16; j++)
        {
            if (i + j < len)
            {
                dbglog("%02x ", buf[i + j]);
            }
            else
            {
                dbglog("   ");
            }
        }
        dbglog(" ");
        for (int j = 0; j < 16; j++)
        {
            if (i + j < len)
            {
                dbglog("%c", isprint(buf[i + j]) ? buf[i + j] : '.');
            }
        }
        dbglog("\n");
    }
}
#endif

ulongint getTickCount()
{
#ifdef __APPLE__
    timeb tb;
    ftime(&tb);		    
    return tb.millitm + (tb.time & 0xfffff) * 1000;
#else
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts))
    {
        return 0;
    }

    return (ts.tv_sec * 1000) + ts.tv_nsec / 1000000L;
#endif
}

void tsAddMsecs(struct timespec* ts, longint millisecs)
{
    int sec = millisecs / 1000;

    millisecs -= sec * 1000;
    ts->tv_nsec += millisecs * 1000000;

    ts->tv_sec += ts->tv_nsec / 1000000000 + sec;
    ts->tv_nsec = ts->tv_nsec % 1000000000;
}

int random(int max)
{
    // Not thread-safe
    static bool seeded = false;
    if (!seeded)
    {
        srand (time(NULL));
        seeded = true;
    }
    return rand() % max;
}

std::string nowStr(const char* fmt /* = NULL */)
{
    Date t;
    t.now();
    return t.toString(fmt);
}


// Buffer::

void Buffer::alloc(size_t size)
{
    free();
    mMemory = ::malloc(size);
    if (mMemory == NULL)
    {
        dbgerr("failed to allocate %lu bytes\n", size);
        return;
    }
    mSize = size;
}

void Buffer::reAlloc(size_t size)
{
    if (size == 0)
    {
        free();
        return;
    }
    void* p = ::realloc(mMemory, size);
    if (p == NULL)
    {
        dbgerr("failed to allocate %lu bytes\n", size);
        return;
    }

    mMemory = p;
    mSize = size;
}

void Buffer::dblOr(size_t neededsize)
{
    size_t newsize = mSize;

    if (newsize == 0)
    {
        newsize = 64;
    }
    else
    {
        newsize *= 2;
    }
    if (newsize < neededsize)
    {
        newsize = neededsize;
    }
    reAlloc(newsize);
}

void Buffer::free()
{
    if (mMemory != NULL)
    {
        ::free(mMemory);
        mMemory = NULL;
        mSize = 0;
    }
}

void Buffer::copyFrom(Buffer& src)
{
    reAlloc(src.size());
    memcpy(ptr(), src.ptr(), src.size());
}

void Buffer::copyFrom(const Buffer& src)
{
    reAlloc(src.size());
    memcpy(ptr(), src.cptr(), src.size());
}

void Buffer::moveFrom(Buffer& src)
{
    free();
    mMemory = src.mMemory;
    mSize = src.mSize;
    src.mMemory = NULL;
    src.mSize = 0;
}

bool Buffer::readFile(const char* filename, bool nullterm)
{
    // Read a file into memory buffer. If asked, a zero is appended at the end
    // so the buffer can be used as a null terminated string.

    FILE* f;
    struct stat st;
    bool ret = false;

    if (stat(filename, &st) == 0)
    {
        f = fopen(filename, "rb");

        reAlloc(st.st_size + (nullterm ? 1 : 0));
        char* buf = (char*)ptr();

        if (buf != NULL)
        {
            if (fread(buf, 1, st.st_size, f) == (size_t)st.st_size)
            {
                if (nullterm)
                {
                    buf[st.st_size] = '\0';
                }
                ret = true;
            }
        }
        fclose(f);
    }

    if (!ret)
    {
        // Failed to read, free the buffer.
        free();
        dbgerr("Failed to read file '%s' into buffer\n", filename);
    }

    return ret;
}

} // jvar
