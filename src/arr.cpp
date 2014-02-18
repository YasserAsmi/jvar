// Copyright (c) 2014 Yasser Asmi
// Released under the MIT License (http://opensource.org/licenses/MIT)

#include "util.h"
#include "var.h"
#include "arr.h"

namespace jvar
{

// BArray

void BArray::useFixedMem(void* memptr, int* countptr, int maxlen)
{
    mBuf.free();

    mMemPtr = memptr;
    mMaxLen = maxlen;
    setFlag(mFlags, FLAG_FIXEDBUF);

    if (mCountPtr != NULL)
    {
        mCountPtr = countptr;
    }
}

void BArray::clear()
{
    //TODO: double check this implementation
    mBuf.free();
    if (isFlagClear(mFlags, FLAG_FIXEDBUF))
    {
        mMaxLen = 0;
    }
    *mCountPtr = 0;
}

void* BArray::insert(int pos, const void* elem)
{
    if (pos > length() || pos < 0)
    {
        dbgerr("BArray cannot insert at %d\n", pos);
        return NULL;
    }

    if (full())
    {
        // Allocate memory.

        if (isFlagClear(mFlags, FLAG_FIXEDBUF))
        {
            int newlen = (mMaxLen == 0) ? 4 : (mMaxLen * 2);
            ensureAlloc(newlen);
        }
        if (full())
        {
            dbgerr("BArray has no room\n");
            return NULL;
        }
    }

    int shift = (length() - pos);

    //dbgtrc("insert memmove(%d, %d, %d)\n", pos + 1, pos, shift);
    if (shift > 0)
    {
        memmove(get(pos + 1), get(pos), mElemSize * shift);
    }

    // If the element was provided copy it into the BArray.

    if (elem)
    {
        memcpy(get(pos), elem, mElemSize);
    }
    (*mCountPtr)++;

    return get(pos);
}

void* BArray::addOrModify(const void* elem, bool modifyfound /*= true */)
{
    int pos;

    // If the item already exists, return it as well as copy the element.
    // Otherwise create a new item and return that.

    if (binSearch(elem, pos))
    {
        if (modifyfound)
        {
            if (elem)
            {
                memcpy(get(pos), elem, mElemSize);
            }
            return get(pos);
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return insert(pos, elem);
    }
}

bool BArray::remove(int pos)
{
    if (pos < 0 || pos >= length())
    {
        dbgerr("BArray cannot delete at %d\n", pos);
        return false;
    }

    int shift = (length() - pos - 1);

    dbgtrc("delete memmove(%d, %d, %d)\n", pos, pos + 1, shift);

    if (shift > 0)
    {
        memmove(get(pos), get(pos + 1), mElemSize * shift);
    }
    (*mCountPtr)--;

    // Free memory.

    if (isFlagClear(mFlags, FLAG_FIXEDBUF))
    {
        if (length() <= (mMaxLen / 2))
        {
            ensureAlloc(length());
        }
    }

    return true;
}

bool BArray::remove(const void* elem)
{
    int pos;
    if (!binSearch(elem, pos))
    {
        return false;
    }
    return remove(pos);
}

void* BArray::find(const void* elem)
{
    int pos;
    if (!binSearch(elem, pos))
    {
        return NULL;
    }
    return get(pos);
}

bool BArray::binSearch(const void* findelem, int& pos)
{
    assert(mComp);
    if (length() == 0 || mComp == NULL)
    {
        pos = 0;
        return false;
    }

    int low = 0;
    int high = length() - 1;
    while (low <= high)
    {
        int mid = (low + high) / 2;
        int res = (*mComp)(get(mid), findelem);

        if (res == 0)
        {
            pos = mid;
            return true;
        }
        else if (res < 0)
        {
            low = mid + 1;
        }
        else
        {
            high = mid - 1;
        }
    }

    pos = low;
    return false;
}

void BArray::ensureAlloc(int desiredlen)
{
    if (mMaxLen == desiredlen || isFlagSet(mFlags, FLAG_FIXEDBUF))
    {
        return;
    }

    mBuf.reAlloc(desiredlen * mElemSize);

    mMemPtr = mBuf.ptr();
    mMaxLen = mBuf.size() / mElemSize;

    //dbgtrc("BArray ensureAlloc %lu bytes for %d elems at %p\n", mBuf.size(), mMaxLen, mMemPtr);
}

void BArray::copyFrom(BArray& src, bool alloconly, bool move)
{
    mFlags = src.mFlags;
    mElemSize = src.mElemSize;
    mComp = src.mComp;
    mCountLocal = src.mCountLocal;
    mMaxLen = src.mMaxLen;

    if (isFlagSet(mFlags, FLAG_FIXEDBUF))
    {
        mMemPtr = src.mMemPtr;
    }
    else
    {
        if (alloconly)
        {
            mBuf.reAlloc(src.mBuf.size());
            mMemPtr = mBuf.ptr();
        }
        else if (move)
        {
            mBuf.moveFrom(src.mBuf);

            src.mMemPtr = NULL;
            src.mMaxLen = 0;
            src.mCountLocal = 0;
        }
        else
        {
            mBuf.copyFrom(src.mBuf);

            mMemPtr = mBuf.ptr();
        }
    }

    if (src.mCountPtr == &src.mCountLocal)
    {
        mCountPtr = &mCountLocal;
    }
    else
    {
        mCountPtr = src.mCountPtr;
    }
}

} // jvar